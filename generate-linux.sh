#!/bin/sh

usage="Usage: $0 [-h|--help] [<options>] [<build-config>]"

wine_mono_ver="10.1.0"
wine_mono_url="https://dl.winehq.org/wine/wine-mono"
msvc_wine_repo="https://github.com/mstorsjo/msvc-wine.git"

msvc_cache_dir="/tmp/msvc"
msvc_install_dir="$HOME/.msvc"
wine_prefix="$HOME/.wine"
build_config="Debug"

owd="$(pwd)"
prog="$0"

fail ()
{
  cd "$owd"
  exit 1
}

diag ()
{
  echo "$*" 1>&2
}

error ()
{
  diag "error: $*"
  fail
}

run ()
{
  diag "+ $@"
  "$@"
  if test "$?" -ne "0"; then
    fail
  fi
}

check_cmd ()
{
  if ! command -v "$1" >/dev/null 2>&1; then
    diag "error: unable to execute $1: command not found"
    if test -n "$2"; then
      diag "  info: $2"
    fi
    fail
  fi
}

get_package_manager ()
{
  if command -v apt >/dev/null 2>&1; then
    echo "apt"
  elif command -v dnf >/dev/null 2>&1; then
    echo "dnf"
  elif command -v yum >/dev/null 2>&1; then
    echo "yum"
  elif command -v pacman >/dev/null 2>&1; then
    echo "pacman"
  else
    echo ""
  fi
}

get_install_cmd ()
{
  case "$1" in
    apt)
      echo "sudo apt install -y"
      ;;
    dnf)
      echo "sudo dnf install -y"
      ;;
    yum)
      echo "sudo yum install -y"
      ;;
    pacman)
      echo "sudo pacman -S --noconfirm"
      ;;
    *)
      echo ""
      ;;
  esac
}

yes=
clean=
skip_deps=
skip_submodules=
debug=true
verbose=
force_full_setup=
timeout=300
connect_timeout=30

while test $# -ne 0; do
  case "$1" in
    -h|--help)
      diag
      diag "$usage"
      diag "Options:"
      diag "  --yes                 Do not ask for confirmation before starting."
      diag "  --clean               Clean build environment before starting."
      diag "  --skip-deps           Skip system dependency installation."
      diag "  --skip-submodules     Skip git submodule update."
      diag "  --release             Build release configuration instead of debug."
      diag "  --verbose             Enable verbose output."
      diag "  --force-full-setup    Force full Wine/MSVC setup even if MSBuild is available."
      diag "  --msvc-cache <dir>    MSVC download cache directory (/tmp/msvc by default)."
      diag "  --msvc-install <dir>  MSVC installation directory (~/.msvc by default)."
      diag "  --wine-prefix <dir>   Wine prefix directory (~/.wine by default)."
      diag "  --timeout <sec>       Network operations timeout in seconds (300 by default)."
      diag
      diag "This script requires root privileges to install system packages."
      diag
      diag "Supported build configurations: Debug, Release"
      diag
      exit 0
      ;;
    --yes)
      yes=true
      shift
      ;;
    --clean)
      clean=true
      shift
      ;;
    --skip-deps)
      skip_deps=true
      shift
      ;;
    --skip-submodules)
      skip_submodules=true
      shift
      ;;
    --release)
      debug=false
      build_config="Release"
      shift
      ;;
    --verbose)
      verbose=true
      shift
      ;;
    --force-full-setup)
      force_full_setup=true
      shift
      ;;
    --msvc-cache)
      shift
      if test $# -eq 0; then
        error "MSVC cache directory expected after --msvc-cache; run $prog -h for details"
      fi
      msvc_cache_dir="$1"
      shift
      ;;
    --msvc-install)
      shift
      if test $# -eq 0; then
        error "MSVC install directory expected after --msvc-install; run $prog -h for details"
      fi
      msvc_install_dir="$1"
      shift
      ;;
    --wine-prefix)
      shift
      if test $# -eq 0; then
        error "Wine prefix directory expected after --wine-prefix; run $prog -h for details"
      fi
      wine_prefix="$1"
      shift
      ;;
    --timeout)
      shift
      if test $# -eq 0; then
        error "value in seconds expected after --timeout; run $prog -h for details"
      fi
      timeout="$1"
      shift
      ;;
    -*)
      diag "error: unknown option '$1'"
      diag "  info: run '$prog -h' for usage"
      fail
      ;;
    *)
      build_config="$1"
      shift
      if test $# -ne 0; then
        diag "error: unexpected argument '$@'"
        diag "  info: options must come before the <build-config> argument"
        fail
      fi
      break
      ;;
  esac
done

case "$build_config" in
  Debug|Release)
    ;;
  *)
    error "invalid build configuration '$build_config' (must be Debug or Release)"
    ;;
esac

if test "$verbose" = true; then
  set -x
fi

prompt_continue ()
{
  while test -z "$yes"; do
    printf "Continue? [y/n] " 1>&2
    read yes
    case "$yes" in
      y|Y) yes=true ;;
      n|N) fail     ;;
        *) yes=     ;;
    esac
  done
}

check_msbuild_available ()
{
  if test "$force_full_setup" = true; then
    return 1
  fi

  if ! test -d "$wine_prefix"; then
    return 1
  fi

  if ! test -d "$msvc_install_dir"; then
    return 1
  fi

  msbuild_path="$msvc_install_dir/bin/x86/msbuild.exe"
  if ! test -f "$msbuild_path"; then
    return 1
  fi

  # Normally we'd check whether MSBuild runs correctly, but its output
  # buffering makes that trickier than it should be. So for now, we'll
  # assume it works.

  return 0
}

download ()
{
  if test -n "$2"; then
    run curl -fL --connect-timeout "$connect_timeout" --max-time "$timeout" --progress-bar -o "$2" "$1"
  else
    run curl -fL --connect-timeout "$connect_timeout" --max-time "$timeout" --progress-bar -O "$1"
  fi
}

install_dependencies ()
{
  if test "$skip_deps" = true; then
    diag "info: skipping system dependency installation"
    return
  fi

  diag "info: detecting package manager..."

  pkg_mgr="$(get_package_manager)"
  if test -z "$pkg_mgr"; then
    error "unable to detect package manager (apt, dnf, yum, pacman)"
  fi

  install_cmd="$(get_install_cmd "$pkg_mgr")"
  if test -z "$install_cmd"; then
    error "unable to determine package installation command for $pkg_mgr"
  fi

  diag "info: detected package manager: $pkg_mgr"

  case "$pkg_mgr" in
    apt)
      packages="wine python3 msitools ca-certificates winbind git curl build-essential"
      ;;
    dnf|yum)
      packages="wine python3 msitools ca-certificates samba-winbind git curl gcc gcc-c++ make"
      ;;
    pacman)
      packages="wine python msitools ca-certificates samba git curl base-devel"
      ;;
  esac

  diag "info: installing system dependencies..."
  run $install_cmd $packages
}

setup_wine ()
{
  diag "info: setting up Wine environment..."

  export WINEPREFIX="$wine_prefix"
  export WINEDLLOVERRIDES="mscoree,mshtml="

  if ! test -d "$wine_prefix"; then
    diag "info: initializing Wine prefix at $wine_prefix"
    run wineboot --init
  fi

  wine_mono_msi="wine-mono-$wine_mono_ver-x86.msi"
  wine_mono_url_full="$wine_mono_url/$wine_mono_ver/$wine_mono_msi"

  if ! test -f "/tmp/$wine_mono_msi"; then
    diag "info: downloading wine-mono $wine_mono_ver..."
    download "$wine_mono_url_full" "/tmp/$wine_mono_msi"
  else
    diag "info: using cached wine-mono installer"
  fi

  diag "info: installing wine-mono..."
  run msiexec /i "/tmp/$wine_mono_msi" /quiet
}

setup_msvc ()
{
  diag "info: setting up MSVC toolchain..."

  if ! test -d "msvc-wine"; then
    diag "info: cloning msvc-wine repository..."
    run git clone "$msvc_wine_repo" msvc-wine
  else
    diag "info: updating msvc-wine repository..."
    run cd msvc-wine
    run git pull
    run cd "$owd"
  fi

  run cd msvc-wine

  if ! test -d "$msvc_install_dir"; then
    diag "info: downloading MSVC toolchain to $msvc_cache_dir..."
    run mkdir -p "$msvc_cache_dir"
    run python3 ./vsdownload.py --accept-license --cache "$msvc_cache_dir" --dest "$msvc_install_dir"
  else
    diag "info: MSVC toolchain already installed at $msvc_install_dir"
  fi

  diag "info: installing MSVC into Wine prefix..."
  run ./install.sh "$msvc_install_dir"

  run cd "$owd"
}

update_submodules ()
{
  if test "$skip_submodules" = true; then
    diag "info: skipping git submodule update"
    return
  fi

  diag "info: updating git submodules..."
  run git submodule update --init --recursive
}

get_revision_number ()
{
  git rev-list --count HEAD 2>/dev/null | tr -d '\n'
}

get_branch_name ()
{
  local branch_name

  branch_name=$(git branch --show-current 2>/dev/null | tr -d '\n')

  if test -z "$branch_name"; then
    branch_name=$(git show -s --pretty=%d HEAD 2>/dev/null | sed -n 's/.*,.*, \([^)]*\).*/\1/p' | tr -d '\n')
  fi

  if test -z "$branch_name"; then
    branch_name="develop"
  fi

  echo "$branch_name"
}

generate_buildinfo ()
{
  diag "info: generating build information..."

  local rev_number=$(get_revision_number)
  local branch_name=$(get_branch_name)
  local old_version="(none)"

  diag "info: detected branch: $branch_name"
  diag "info: detected revision: $rev_number"

  if test -f "src/version.h"; then
    old_version=$(grep "#define REVISION " src/version.h 2>/dev/null | awk '{print $3}' | tr -d '\n')
    if test -z "$old_version"; then
      old_version="(none)"
    fi
  fi

  if test "$old_version" != "$rev_number"; then
    diag "info: updating version $old_version -> $rev_number"

    cat > src/version.h << EOF
/*
 * Automatically generated by bootstrap script.
 * Do not touch!
 */

#define GIT_BRANCH "$branch_name"

// Revision definition
#define REVISION $rev_number
#define REVISION_STR "r$rev_number"

EOF
    if test "$branch_name" = "develop"; then
      cat >> src/version.h << EOF
// Branch-specific definitions
#define EXPERIMENTAL_BUILD
EOF
    fi

    # Generate version.hpp
    cat > src/version.hpp << EOF
/*
 * Automatically generated by bootstrap script.
 * Do not touch!
 *
 * This file exists for reasons of complying with our coding standards.
 *
 * The Resource Compiler will ignore any content from C++ header files if they're not from STDInclude.hpp.
 * That's the reason why we now place all version info in version.h instead.
 */

#include "version.h"
EOF
  else
    diag "info: version information up to date"
  fi
}

generate_vscode_config ()
{
  diag "info: generating VSCode configuration..."

  local workspace_folder=$(pwd)

  if ! test -d ".vscode"; then
    run mkdir -p .vscode
  fi

	local msvc_version=""
  if test -d "$msvc_install_dir/vc/tools/msvc"; then
    msvc_version=$(ls "$msvc_install_dir/vc/tools/msvc" | sort -V | tail -n 1)
  fi

  local sdk_version=""
  if test -d "$msvc_install_dir/kits/10/Include"; then
    sdk_version=$(ls "$msvc_install_dir/kits/10/Include" | grep -E '^10\.' | sort -V | tail -n 1)
  fi

  # Fallback to known versions if version detection fails
	#
  if test -z "$msvc_version"; then
    msvc_version="14.44.35207"
    diag "warning: could not detect MSVC version, using fallback: $msvc_version"
  else
    diag "info: detected MSVC version: $msvc_version"
  fi

  if test -z "$sdk_version"; then
    sdk_version="10.0.26100.0"
    diag "warning: could not detect Windows SDK version, using fallback: $sdk_version"
  else
    diag "info: detected Windows SDK version: $sdk_version"
  fi

  cat > .vscode/c_cpp_properties.json << EOF
{
	"configurations": [
		{
			"name": "Linux",
			"includePath": [
				"\${default}",
				"$msvc_install_dir/vc/tools/msvc/$msvc_version/atlmfc/include",
				"$msvc_install_dir/vc/tools/msvc/$msvc_version/include",
				"$msvc_install_dir/kits/10/Include/$sdk_version/shared",
				"$msvc_install_dir/kits/10/Include/$sdk_version/ucrt",
				"$msvc_install_dir/kits/10/Include/$sdk_version/um",
				"$msvc_install_dir/kits/10/Include/$sdk_version/winrt",
				"\${workspaceFolder}/**"
			],
			"defines": [],
			"cStandard": "c23",
			"cppStandard": "c++23",
			"intelliSenseMode": "windows-msvc-x86",
			"forcedInclude": [
				"$workspace_folder/src/STDInclude.hpp"
			]
		}
	],
	"version": 4
}
EOF

  diag "info: VSCode configuration generated at .vscode/c_cpp_properties.json"
  diag "info: Using MSVC installation directory: $msvc_install_dir"
}

strip_premake_build_commands ()
{
  diag "info: creating stripped premake5 configuration..."

  # Strip lines 247-274 (pre-build and post-build commands)
  #
  # NOTE: Update if changing anything in premake5.lua!
  #
  sed '247,274d' premake5.lua > premake5-linux.lua
}

cleanup_premake_temp_files ()
{
  diag "info: cleaning up temporary premake files..."

  if test -f "premake5-linux.lua"; then
    run rm -f premake5-linux.lua
  fi
}

generate_build_files ()
{
  diag "info: generating build files..."

  if ! test -f "tools/premake5.exe"; then
    error "premake5 executable not found in tools/ directory"
  fi

  generate_buildinfo
  generate_vscode_config
  strip_premake_build_commands

  run wine tools/premake5.exe --file=premake5-linux.lua vs2022

  cleanup_premake_temp_files
}

build_project ()
{
  diag "info: building IW4x client ($build_config configuration)..."

  solution_file="build/iw4x.sln"
  if ! test -f "$solution_file"; then
    error "solution file $solution_file not found; generation may have failed"
  fi

  msbuild_args="/p:Configuration=$build_config"
  if test "$verbose" = true; then
    msbuild_args="$msbuild_args /verbosity:detailed"
  fi

  run "$msvc_install_dir/bin/x86/msbuild.exe" "$solution_file" $msbuild_args
}

clean_environment ()
{
  if test "$clean" = true; then
    diag "info: cleaning build environment..."
    run rm -rf build/
    run rm -rf msvc-wine/
    run rm -rf "$wine_prefix"
    run rm -rf "$msvc_install_dir"
    run rm -f /tmp/wine-mono-*.msi
  fi
}

install ()
{
  if check_msbuild_available; then
    diag
    diag "-------------------------------------------------------------------------"
    diag
    diag "MSBuild environment detected! Using fast path."
    diag
    diag "Build configuration: $build_config"
    diag "Existing Wine prefix: $wine_prefix"
    diag "Existing MSVC dir:    $msvc_install_dir"
    diag
    diag "This will:"
    if test "$skip_submodules" != true; then
    diag "  1. Update git submodules"
    diag "  2. Generate VSCode configuration"
    diag "  3. Regenerate Visual Studio solution files"
    diag "  4. Build IW4x client using existing MSBuild"
    else
    diag "  1. Generate VSCode configuration"
    diag "  2. Regenerate Visual Studio solution files"
    diag "  3. Build IW4x client using existing MSBuild"
    fi
    diag
    diag "To force full setup instead, use --force-full-setup option."
    diag

    prompt_continue

    update_submodules
    generate_build_files
    build_project

    diag
    diag "-------------------------------------------------------------------------"
    diag
    diag "Successfully rebuilt IW4x client using fast path!"
    diag
    diag "Build configuration: $build_config"
    diag "Output files can be found in: build/"
    diag
    diag "Build time was significantly reduced by reusing existing environment."
    diag
    return
  fi

  diag
  diag "-------------------------------------------------------------------------"
  diag
  diag "About to bootstrap IW4x build environment on Linux (full setup)."
  diag
  diag "Build configuration: $build_config"
  diag "Wine prefix:         $wine_prefix"
  diag "MSVC install dir:    $msvc_install_dir"
  diag "MSVC cache dir:      $msvc_cache_dir"
  diag
  diag "This will:"
  if test "$skip_deps" != true; then
  diag "  1. Install system dependencies (wine, python3, msitools, etc.)"
  diag "  2. Set up Wine environment and install wine-mono"
  diag "  3. Download and install MSVC toolchain via msvc-wine"
  if test "$skip_submodules" != true; then
  diag "  4. Update git submodules"
  diag "  5. Generate VSCode configuration"
  diag "  6. Generate Visual Studio solution files"
  diag "  7. Build IW4x client"
  else
  diag "  4. Generate VSCode configuration"
  diag "  5. Generate Visual Studio solution files"
  diag "  6. Build IW4x client"
  fi
  else
  diag "  1. Set up Wine environment and install wine-mono"
  diag "  2. Download and install MSVC toolchain via msvc-wine"
  if test "$skip_submodules" != true; then
  diag "  3. Update git submodules"
  diag "  4. Generate VSCode configuration"
  diag "  5. Generate Visual Studio solution files"
  diag "  6. Build IW4x client"
  else
  diag "  3. Generate VSCode configuration"
  diag "  4. Generate Visual Studio solution files"
  diag "  5. Build IW4x client"
  fi
  fi
  diag

  if test "$clean" = true; then
    diag "WARNING: --clean option will remove existing Wine prefix and MSVC installation."
    diag
  fi

  prompt_continue

  clean_environment

  install_dependencies

  check_cmd wine "install wine package for your distribution"
  check_cmd git "install git package for your distribution"
  check_cmd curl "install curl package for your distribution"
  check_cmd python3 "install python3 package for your distribution"
  check_cmd msiexec "wine installation may be incomplete"

  setup_wine
  setup_msvc
  update_submodules
  generate_build_files
  build_project

  diag
  diag "-------------------------------------------------------------------------"
  diag
  diag "Successfully built IW4x client!"
  diag
  diag "Build configuration: $build_config"
  diag "Output files can be found in: build/"
  diag
  if test "$build_config" = "Debug"; then
    diag "Debug build completed. Use this for development and debugging."
  else
    diag "Release build completed. This is optimized for production use."
  fi
  diag
  diag "To rebuild in the future, you can run:"
  diag "  $prog --yes $build_config"
  diag "or manually with:"
  diag "  wine \"$msvc_install_dir/bin/x86/msbuild.exe\" build/iw4x.sln /p:Configuration=$build_config"
  diag
}

install
