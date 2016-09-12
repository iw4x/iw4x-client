#!groovy

/*
This is our new pipeline script to do all of the building in, of and around IW4x.

Here's what it is supposed to do:

- Make sure Modern Warfare 2 is installed (CI should provide the folder like a custom tool)
- Check out code from iw4x-data
- Build the IW4x client library (this code repository)
- Use iw4x.exe from the iw4x-data repository in order to build the zone files in iw4x-data
- Package the IW4x client with the newly built data files

At this point it is done building everything, however afterwards we want the build server to
also push the newly built files to an update repository, depending on the branch we're on.

- For "develop", release to the "iw4x-dev" branch on the repository server.
- For "master", release to the "iw4x" branch on the repository server.

I'm looking into how the logic of pipelining works in detail before deciding on whether to
throw in the IW4x Updater and the IW4x Node binaries in as well or not.
*/

/*
Note that this is just a rewrite of the jobs as they are currently set up on the production
Jenkins server. This will allow every developer to tinker around with how the build process
is set up. For those who want to play around with this, here's a bit of information:

- This is a Groovy script. Essentially Java but with less bullshit (like brackets and verbose writing).
- This gets directly translated into a Jenkins pipeline.
- If you have no idea how to handle scripts, get your hands off this file.
- If you do not use Jenkins, get your hands off this file.
- If you fuck this script up, I will kill you.
*/

@Field def configurations = [
	"Debug",
	"DebugStatic",
	"Release",
	"ReleaseStatic"
]

// Run a function for each platform and project to be built
def perConfiguration(suffix, f) {
	def executions = [:]
	for (int a = 0; a < configurations.size(); a++) {
		def configuration = configurations[a]
		executions["$configuration$suffix"] = {
			f(goos, goarch, targetProject)
		}
	}
	parallel executions
}

// This will build the IW4x client.
// We need a Windows Server with Visual Studio 2015, Premake5 and Git on it.
def doBuild(premakeFlags, configuration) {
	node("windows") {
		sshagent (credentials: ["ba9ec261-deff-4fa0-a0e8-5d755f88d035"]) {
			checkout scm

			premakeHome = "${pwd()}\\src\\tools"

			withEnv(["PATH+=${premakeHome}"]) {
				def outputDir = pwd()
				dir("src") {
					bat "premake5 vs2015 $premakeFlags"
					bat "\"${tool 'MSBuild'}\" src\\build\\iw4x.sln \"/p:OutDir=$outputDir\\\" \"/p:Configuration=$configuration\""
				}
			}

			archiveArtifacts artifacts: "*", fingerprint: true
			stash name: "iw4x $configuration", includes: "*"
		}

	}
}

// For each available configuration generate a normal build and a unit test build.
stage "Build"
def executions = [:]
for (int i = 0; i < configurations.size(); i++)
{
	configuration = configurations[i]
	executions["$configuration"] = {
		doBuild premakeFlags: "", configuration: configuration
	}
	executions["$configuration Unit-Testing"] = {
		doBuild premakeFlags: "--force-unit-tests", configuration: configuration
	}
}
parallel executions

