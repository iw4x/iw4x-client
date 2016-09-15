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

import groovy.transform.Field

@Field def configurations = [
	"Debug",
	"DebugStatic",
	"Release",
	"ReleaseStatic"
]

def useShippedPremake(f) {
	def premakeHome = "${pwd()}\\tools"

	withEnv(["PATH+=${premakeHome}"], f)
}

def getIW4xExecutable() {
	step([
		$class: 'CopyArtifact',
		filter: '*',
		fingerprintArtifacts: true,
		projectName: 'iw4x/iw4x-executable/master',
		selector: [
			$class: 'TriggeredBuildSelector',
			allowUpstreamDependencies: false,
			fallbackToLastSuccessful: true,
			upstreamFilterStrategy: 'UseGlobalSetting'
		]
	])
}

// This will build the IW4x client.
// We need a Windows Server with Visual Studio 2015, Premake5 and Git on it.
def doBuild(name, wsid, premakeFlags, configuration) {
	node("windows") {
		ws("IW4x/build/$wsid") {
			checkout scm

			useShippedPremake {
				def outputDir = pwd()
				def msbuild = tool "Microsoft.NET MSBuild 14.0"
				bat "premake5 vs2015 $premakeFlags"
				bat "\"${msbuild}\" build\\iw4x.sln \"/p:OutDir=$outputDir\\\\\" \"/p:Configuration=$configuration\""
			}

			stash name: "$name", includes: "*.dll,*.pdb"
		}
	}
}

// This will run the unit tests for IW4x.
// We need a Windows Server with MW2 on it.
def doUnitTests(label, name, wsid) {
	ws("IW4x/testing/$wsid") {
		mw2dir = tool "Modern Warfare 2"

		unstash "$name"

		// Get installed localization for correct zonefiles directory junction
		def localization = readFile("$mw2dir/localization.txt").split("\r?\n")[0]

		try {
			timeout(time: 180, unit: "MINUTES") {
				// Set up environment
				if (isUnix()) {
					sh """
					mkdir -p zone
					for f in main zone/dlc \"zone/$localization\"; do
						ln -sfv \"$mw2dir/\$f\" \"\$f\"
					done
					for f in \"$mw2dir\"/*.dll \"$mw2dir\"/*.txt \"$mw2dir\"/*.bmp; do
						ln -sfv \"\$f\" \"\$(basename \"\$f\")\"
					done
					"""
				} else {
					bat """
					mklink /J \"main\" \"$mw2dir\\main\"
					mkdir \"zone\"
					mklink /J \"zone\\dlc\" \"$mw2dir\\zone\\dlc\"
					mklink /J \"zone\\$localization\" \"$mw2dir\\zone\\$localization\"
					copy /y \"$mw2dir\\*.dll\"
					copy /y \"$mw2dir\\*.txt\"
					copy /y \"$mw2dir\\*.bmp\"
					"""
				}

				// Run tests
				getIW4xExecutable()
				if (isUnix()) {
					sh "wine-wrapper iw4x.exe -tests"
				} else {
					bat "iw4x.exe -tests"
				}
			}
		} finally {
			// In all cases make sure to at least remove the directory junctions!
			if (!isUnix()) {
				bat """
				rmdir \"main\"
				rmdir \"zone\\dlc\"
				rmdir \"zone\\$localization\"
				"""
			}
			deleteDir()
		}
	}
}

// Job properties
properties([
	[$class: "GitLabConnectionProperty", gitLabConnection: "sr0"]
])

gitlabBuilds(builds: ["Checkout & Versioning", "Build", "Testing", "Archiving"]) {
	// First though let's give this build a proper name
	stage("Checkout & Versioning") {
		gitlabCommitStatus("Checkout & Versioning") {
			node("windows") {
				checkout scm

				useShippedPremake {
					def version = bat(returnStdout: true, script: '@premake5 version').split("\r?\n")[1]

					currentBuild.setDisplayName "$version (#${env.BUILD_NUMBER})"
				}
			}
		}
	}

	// For each available configuration generate a normal build and a unit test build.
	stage("Build") {
		gitlabCommitStatus("Build") {
			def executions = [:]
			for (int i = 0; i < configurations.size(); i++)
			{
				def configuration = configurations[i]
				executions["$configuration"] = {
					doBuild("IW4x $configuration", "$configuration", "", configuration)
				}
				executions["$configuration with unit tests"] = {
					doBuild("IW4x $configuration (unit tests)", "$configuration+unittests", "--force-unit-tests", configuration)
				}
			}
			parallel executions
		}
	}

	// Run unit tests on each configuration.
	stage("Testing") {
		gitlabCommitStatus("Testing") {
			executions = [:]
			for (int i = 0; i < configurations.size(); i++)
			{
				def configuration = configurations[i]
				executions["$configuration on Windows"] = {
					node("windows") {
						doUnitTests("IW4x $configuration (unit tests)", configuration)
					}
				}
				executions["$configuration on Linux"] = {
					node("docker && linux && amd64") {
						docker.build("github.com/IW4x/iw4x-client-testing-wine32", "--rm --force-rm -f wine32.Dockerfile jenkins").inside {
							doUnitTests("IW4x $configuration (unit tests)", configuration)
						}
					}
				}
			}
			parallel executions
		}
	}

	// Collect all the binaries and give each configuration its own subfolder
	stage("Archiving") {
		gitlabCommitStatus("Archiving") {
			node("windows") { // any node will do
				ws("IW4x/pub") {
					try {
						for (int i = 0; i < configurations.size(); i++)
						{
							def configuration = configurations[i]
							dir("$configuration") {
								unstash "IW4x $configuration"
							}
						}
						archiveArtifacts artifacts: "**/*.dll,**/*.pdb", fingerprint: true
					} finally {
						deleteDir()
					}
				}
			}
		}
	}
}