# Copyright (c) 2016 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# ============================================================================
#
# Author(s):
#   Stefan Wallentowitz <stefan@wallentowitz.de>
#

from .interface import *

class Module(object):
	modid = None
	type = None
	def __init__(self, id, type):
		self.modid = id
		self.type = type
	def create(id, type):
		if type == "MAM":
			return Memory(id)
		elif type == "STM":
			return STM(id)
		elif type == "CTM":
			return CTM(id)
		else:
			return Module(id, type)
	create = staticmethod(create)

	def get_type(self):
		return self.type
	def get_id(self):
		return self.modid

class Memory(Module):
	def __init__(self, id):
		super(Memory, self).__init__(id, "MAM")

	def loadelf(self, filename, verify = False):
		return python_osd_mem_loadelf(self.modid, filename, verify)

class STM(Module):
	def __init__(self, id):
		super(STM, self).__init__(id, "STM")
	def log(self, filename):
		python_osd_stm_log(self.modid, filename)

class CTM(Module):
	def __init__(self, id):
		super(CTM, self).__init__(id, "CTM")
	def log(self, filename, elffile = None):
		if elffile:
			python_osd_ctm_log_symbols(self.modid, filename, elffile)
		else:
			python_osd_ctm_log(self.modid, filename)

class Session(object):
	memories = []
	modules = []

	def __init__(self):
		for i in range(0,python_osd_get_num_modules()):
			m = Module.create(i, python_osd_get_module_name(i))
			self.modules.append(m)

	def reset(self, halt=False):
		python_osd_reset(halt)

	def get_memories(self):
		return self.memories

	def get_modules(self,type=None):
		if type == None:
			return self.modules
		else:
			mods = []
			for m in self.modules:
				if m.get_type() == type:
					mods.append(m)
			return mods

	def start(self):
		python_osd_start()

	def wait(self, secs):
		python_osd_wait(secs)
