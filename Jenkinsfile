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
	"Debug": [
		WorkspaceID: "build@debug",
		StashName: "iw4x-debug",
		MSBuildConfiguration: "Debug",
		PremakeArgs: "",
		Archive: true,
	],
	"Release": [
		WorkspaceID: "build@release",
		StashName: "iw4x-release",
		MSBuildConfiguration: "Release",
		PremakeArgs: "",
		Archive: true,
	],
	"Release with unit tests": [
		WorkspaceID: "build@release+unittests",
		StashName: "iw4x-release-unittests",
		MSBuildConfiguration: "Release",
		PremakeArgs: "--force-unit-tests",
		Archive: false,
	],
].collect {k, v -> [k, v]}

@Field def testing = [
	"Debug": [
		WorkspaceID: "testing@debug",
		StashName: "iw4x-debug",
	],
	"Release": [
		WorkspaceID: "testing@release",
		StashName: "iw4x-release-unittests",
	],
].collect {k, v -> [k, v]}

def jobWorkspace(id, f) {
	ws("workspace/${env.JOB_NAME.replaceAll(/[%$]/, "_")}@$id", f)
}

def useShippedPremake(f) {
	def premakeHome = "${pwd()}\\tools"

	withEnv(["PATH+=${premakeHome}"], f)
}

def getIW4xExecutable() {
	step([
		$class: 'CopyArtifact',
		filter: '*',
		fingerprintArtifacts: true,
		projectName: 'iw4x/iw4x-executable/' + iw4xExecutableBranch(),
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
def doBuild(cfg) {
	retry(5) {
		checkout scm
	}

	useShippedPremake {
		def outputDir = pwd()
		def msbuild = tool "Microsoft.NET MSBuild 14.0"
		bat "premake5 vs2015 ${cfg.PremakeArgs}"
		bat "\"${msbuild}\" build\\iw4x.sln \"/p:OutDir=$outputDir\\\\\" \"/p:Configuration=${cfg.MSBuildConfiguration}\""
	}

	stash name: "${cfg.StashName}", includes: "*.dll,*.pdb"
}

// This will run the unit tests for IW4x.
// We need a Windows Server with MW2 on it.
def doUnitTests(name) {
	mw2dir = tool "Modern Warfare 2"

	unstash "$name"

	// Get installed localization for correct zonefiles directory junction
	def localization = readFile("${tool "Modern Warfare 2"}/localization.txt").split("\r?\n")[0]

	try {
		timeout(time: 10, unit: "MINUTES") {
			// Set up environment
			if (isUnix()) {
                def mw2dir = tool "Modern Warfare 2"
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
                def mw2dir = tool "Modern Warfare 2"
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
			retry(2) {
				if (isUnix()) {
					sh "WINEDEBUG=warn+all wine-wrapper iw4x.exe -tests"
				} else {
					bat "iw4x.exe -tests"
				}
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

// Returns the IW4x executable branch to use
def iw4xExecutableBranch() {
	try {
		return IW4X_EXECUTABLE_BRANCH;
	} catch(MissingPropertyException) {
		return "master";
	}
}

// Job properties
properties([
	[$class: "GitLabConnectionProperty", gitLabConnection: "sr0"],
	buildDiscarder(logRotator(artifactDaysToKeepStr: '', artifactNumToKeepStr: '', daysToKeepStr: '', numToKeepStr: '30'))
])

gitlabBuilds(builds: ["Checkout & Versioning", "Build", "Testing", "Archiving"]) {
	// First though let's give this build a proper name
	stage("Checkout & Versioning") {
		gitlabCommitStatus("Checkout & Versioning") {
			node("windows") {
				jobWorkspace("versioning") {
					if (env.BRANCH_NAME == 'master')
					{
						echo 'Reset build environment'
						deleteDir()
					}

					retry(5) {
						checkout scm
					}

					useShippedPremake {
						def version = bat(returnStdout: true, script: '@premake5 version').split("\r?\n")[1]

						currentBuild.setDisplayName "$version (#${env.BUILD_NUMBER})"
					}

					stash name: "jenkins-files", includes: "jenkins/**"
				}
			}
		}
	}

	// For each available configuration generate a normal build and a unit test build.
	stage("Build") {
		gitlabCommitStatus("Build") {
			def executions = [:]
			for (int i = 0; i < configurations.size(); i++) {
				def entry = configurations[i]

				def configName = entry[0]
				def config = entry[1]

				executions[configName] = {
					node("windows") {
						jobWorkspace(config.WorkspaceID) {
							doBuild(config)
						}
					}
				}
			}
			parallel executions
		}
	}

	// Run unit tests on each configuration.
	stage("Testing") {
		gitlabCommitStatus("Testing") {
			executions = [:]
			for (int i = 0; i < testing.size(); i++) {
				def entry = testing.get(i)

				def testName = entry[0]
				def test = entry[1]

				executions["$testName on Windows"] = {
					node("windows") {
						jobWorkspace(test.WorkspaceID) {
							doUnitTests(test.StashName)
						}
					}
				}
				executions["$testName on Linux"] = {
					node("docker && linux && amd64") {
						wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'XTerm']) {
							try {
								def image = null
								dir("src") {
									unstash "jenkins-files"
									image = docker.build("github.com/IW4x/iw4x-client-testing-wine32", "--rm --force-rm -f jenkins/wine32.Dockerfile jenkins")
									deleteDir()
								}
								image.inside {
									doUnitTests(test.StashName)
								}
							}
						}
					}
				}
				parallel executions
			}
		}
	}

	// Collect all the binaries and give each configuration its own subfolder
	stage("Archiving") {
		gitlabCommitStatus("Archiving") {
			node("windows") { // any node will do
				jobWorkspace("archiving") {
					try {
						for (int i = 0; i < configurations.size(); i++) {
							def entry = configurations[i]

							def configName = entry[0]
							def config = entry[1]

							if (config.Archive) {
								dir(configName) {
									unstash config.StashName
								}
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
