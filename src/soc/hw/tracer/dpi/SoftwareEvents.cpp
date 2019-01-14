#include <SoftwareEvents.h>

/*
SOFTWARE_EVENT_BEGIN_DECL(0x0f, reset)
SOFTWARE_EVENT_END_DECL(0x0f, reset)

SOFTWARE_EVENT_BEGIN_DECL(0x10, exception_enter)
EVENT_FIELD(1, cause)
EVENT_TSDL("        enum : integer { size = 8; } {\n"
           "            BUS_ERROR = 2,\n"
           "            DATA_PAGE_FAULT = 3,\n"
           "            INSTRUCTION_PAGE_FAULT = 4,\n"
           "            TICK_TIMER = 5,\n"
           "            ALIGNMENT = 6,\n"
           "            ILLEGAL_INSTRUCTION = 7,\n"
           "            INTERRUPT = 8,\n"
           "            DATA_TLB_MISS = 9,\n"
           "            INSTRUCTION_TLB_MISS = 10,\n"
           "            RANGE = 11,\n"
           "            SYSTEM_CALL = 12,\n"
           "            TRAP = 13,\n"
           "        } cause;\n")

SOFTWARE_EVENT_END_DECL(0x10, exception_enter)

SOFTWARE_EVENT_BEGIN_DECL(0x11, exception_leave)
SOFTWARE_EVENT_END_DECL(0x11, exception_leave)
*/
