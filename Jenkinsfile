node {

    stage('Code Checkout') {
        checkout([$class: 'GitSCM',
          branches: [[name: '*/dex_main']],
                     doGenerateSubmoduleConfigurations: true,
                     extensions: scm.extensions + [[$class: 'SubmoduleOption', 
                                                    parentCredentials: false,
                                                    disableSubmodules: false,
                                                    recursiveSubmodules: true]],
                     userRemoteConfigs: scm.userRemoteConfigs])
    }
    stage('Build') {
    sh """
      make -j8
      export WD=$(pwd)
      env
    """

//     REPORT_FILE=$(find * | grep Test.xml)

//         script {
//               env.TESTFILE = sh(returnStdout: true, script: "find * | grep Test.xml")
//               echo "TESTFILE: ${env.TESTFILE}"
//         }

//         sh "make -j8"
//         sh "ctest -T test --no-compress-output"
//         def foldername = pwd()
//         print "${foldername}"
//         sh 'ls /var/jenkins_home/workspace/cutcoin-cli/build/Linux'
    }
//     stage('Tests') {
//       steps {
//         sh "make release-test"
//         sh 'ctest -T test --no-compress-output'
//       }
//     }

//   post {
//     always {
//       // Archive the CTest xml output
//       archiveArtifacts (
//         artifacts: 'build/Linux/**/**/**/**/*.xml',
//         fingerprint: true
//       )
//
//       // Process the CTest xml output with the xUnit plugin
//       xunit (
//         testTimeMargin: '3000',
//         thresholdMode: 1,
//         thresholds: [
//           skipped(failureThreshold: '0'),
//           failed(failureThreshold: '0')
//         ],
//       tools: [CTest(
//           pattern: 'build/Linux/**/**/**/**/*.xml',
//           deleteOutputFiles: true,
//           failIfNotNew: false,
//           skipNoTestFiles: true,
//           stopProcessingIfError: true
//         )]
//       )
//
//       // Clear the source and build dirs before next run
//       deleteDir()
//     }
//   }
}