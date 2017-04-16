node('fpga-nexys4ddr') {
  stage("Checkout") {
      checkout scm
  }

   stage('Simulation Build') {
     sh_with_modules (
       modules: ["eda/xilinx/vivado/2016.4",
                 "eda/fusesoc/1.6.1",
                 "eda/verilator/3.900",
                 "compilers/openrisc/gcc/5.3.0-newlib2.3.0-1",
                 "base/python/3.5"],
       command: 'make BUILD_DOCS=no BUILD_EXAMPLES_FPGA=no'
     )
   }
   stage('Simulation Tests') {
     sh_with_modules (
       modules: ["eda/xilinx/vivado/2016.4",
                 "eda/fusesoc/1.6.1",
                 "eda/verilator/3.900",
                 "compilers/openrisc/gcc/5.3.0-newlib2.3.0-1",
                 "base/python/3.5"],
       command: 'source objdir/dist/optimsoc-environment.sh; pytest -s -v test/systemtest/test_tutorial.py::TestTutorial'
     )
   }
   stage('FPGA Build') {
     sh_with_modules (
       modules: ["eda/xilinx/vivado/2016.4",
                 "eda/fusesoc/1.6.1",
                 "eda/verilator/3.900",
                 "compilers/openrisc/gcc/5.3.0-newlib2.3.0-1",
                 "base/python/3.5"],
       command: 'make BUILD_DOCS=no'
     )
   }
   stage('FPGA Tests') {
     sh "echo 'boards:' > localconf.yaml"
     sh "echo '  nexys4ddr:' >> localconf.yaml"
     sh "echo '    device: /dev/ttyUSB1' >> localconf.yaml"
     
     sh_with_modules (
       modules: ["eda/xilinx/vivado/2016.4",
                 "eda/fusesoc/1.6.1",
                 "eda/verilator/3.900",
                 "compilers/openrisc/gcc/5.3.0-newlib2.3.0-1",
                 "base/python/3.5"],
       command: 'source objdir/dist/optimsoc-environment.sh; export OPTIMSOC_TEST_LOCALCONF=$PWD/localconf.yaml; cat $OPTIMSOC_TEST_LOCALCONF; pytest -s -v test/systemtest/test_tutorial.py::TestTutorialFpga'
     )
   }  
}
