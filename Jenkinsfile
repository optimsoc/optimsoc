@Library('librecoresci') import org.librecores.ci.Modules
def lcci = new Modules(steps)

node('fpga-nexys4ddr') {
  lcci.load(["eda/xilinx/vivado/2016.4",
             "eda/fusesoc/1.6.1",
             "eda/verilator/3.902",
             "compilers/openrisc/gcc/5.3.0-newlib2.3.0-1",
             "base/python/3.5"])
  
  stage("Checkout") {
      checkout scm
  }

  stage('Simulation Build') {
    lcci.sh 'make BUILD_DOCS=no BUILD_EXAMPLES_FPGA=no'
  }
  
  stage('Simulation Tests') {
     lcci.sh 'source objdir/dist/optimsoc-environment.sh; pytest -s -v test/systemtest/test_tutorial.py::TestTutorial'
  }
  
  stage('FPGA Build') {
    lcci.sh 'make BUILD_DOCS=no'
  }
  
  stage('FPGA Tests') {
    sh "echo 'boards:' > localconf.yaml"
    sh "echo '  nexys4ddr:' >> localconf.yaml"
    sh "echo '    device: /dev/ttyUSB1' >> localconf.yaml"
     
    lcci.sh 'source objdir/dist/optimsoc-environment.sh; export OPTIMSOC_TEST_LOCALCONF=$PWD/localconf.yaml; cat $OPTIMSOC_TEST_LOCALCONF; pytest -s -v test/systemtest/test_tutorial.py::TestTutorialFpga'
  }  
}
