# Cocotb mock objects for Sphinx

Sphinx autodoc works by importing documented Python code. This requires all 
imports to be defined. Installing cocotb is tricky if used only for getting 
documentation builds working. Hence we need to mock cocotb to make the autodoc 
happy. The majority of work is done by the setting `autodoc_mock_imports` in 
`conf.py`. Only decorators cannot be mocked automatically, therefore we manually 
mock them. These manual mocks are contained in this directory.
