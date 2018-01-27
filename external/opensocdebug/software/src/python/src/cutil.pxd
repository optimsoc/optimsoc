# Copyright 2017-2018 The Open SoC Debug Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# these defines are not part of the includes as provided by cython today
cdef extern from "<stdio.h>" nogil:
    ctypedef void* va_list
    int vasprintf(char **strp, const char* fmt, va_list ap)

cdef extern from  "<Python.h>" nogil:
    int Py_AddPendingCall(int (*func)(void *), void *arg)
