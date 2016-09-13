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

// This will build the IW4x client.
// We need a Windows Server with Visual Studio 2015, Premake5 and Git on it.
def doBuild(name, premakeFlags, configuration) {
	node("windows") {
		checkout scm

		premakeHome = "${pwd()}\\tools"

		withEnv(["PATH+=${premakeHome}"]) {
			def outputDir = pwd()
			def msbuild = tool "Microsoft.NET MSBuild 14.0"
			bat "premake5 vs2015 $premakeFlags"
			bat "\"${msbuild}\" build\\iw4x.sln \"/p:OutDir=$outputDir\\\\\" \"/p:Configuration=$configuration\""
		}

		archiveArtifacts artifacts: "*.dll,*.pdb", fingerprint: true
		stash name: "$name", includes: "*.dll,*.pdb"
	}
}

// This will run the unit tests for IW4x.
// We need a Windows Server with MW2 on it.
def doUnitTests(name) {
	node("windows") {
		checkout scm

		mw2dir = tool "Modern Warfare 2"

		unstash "$name"

		// Get installed localization for correct zonefiles directory junction
		def localization = readFile("$mw2dir\\localization.txt").split("\r?\n")[0]

		try {
			timeout(time: 180, unit: "MINUTES") {
				// Set up environment
				bat """
				mklink /J \"main\" \"$mw2dir\\main\"
				mklink /J \"zone\\dlc\" \"$mw2dir\\zone\\dlc\"
				mklink /J \"zone\\$localization\" \"$mw2dir\\zone\\$localization\"
				copy /y \"$mw2dir\\*.dll\"
				copy /y \"$mw2dir\\*.txt\"
				copy /y \"$mw2dir\\*.bmp\"
				"""

				// Run tests
				bat "iw4x.exe -tests"
			}
		} finally {
			// In all cases make sure to at least remove the directory junctions!
			bat """
			rmdir \"main\"
			rmdir \"zone\\dlc\"
			rmdir \"zone\\$localization\"
			"""
		}
	}
}

// Change build name to correct version
stage "Versioning"
node("windows") {
	checkout scm

	version = sh(returnStdout: true, script: 'premake5 version').split("\r?\n")[1]

	currentBuild.setDisplayName "$version (#${env.BUILD_NUMBER})"
}

// For each available configuration generate a normal build and a unit test build.
stage "Build"
def executions = [:]
for (int i = 0; i < configurations.size(); i++)
{
	def configuration = configurations[i]
	executions["$configuration"] = {
		doBuild("IW4x $configuration", "", configuration)
	}
	executions["$configuration with unit tests"] = {
		doBuild("IW4x $configuration (unit tests)", "--force-unit-tests", configuration)
	}
}
parallel executions

// Run unit tests on each configuration.
stage "Testing"
def executions = [:]
for (int i = 0; i < configurations.size(); i++)
{
	def configuration = configurations[i]
	executions["$configuration"] = {
		doUnitTests("IW4x $configuration with unit tests")
	}
}
parallel executions
