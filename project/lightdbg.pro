TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../src/Main.cpp \
    ../src/DebugLoop.cpp \
    ../src/commands/Command.cpp \
    ../src/commands/CommandContinue.cpp \
    ../src/commands/CommandDetach.cpp \
    ../src/commands/CommandExit.cpp \
    ../src/commands/CommandStep.cpp \
    ../src/commands/CommandRegisters.cpp \
    ../src/commands/CommandTracer.cpp \
    ../src/Mnemonic.cpp \
    ../src/Tracer.cpp \
    ../src/commands/CommandDisassemble.cpp \
    ../src/commands/CommandObfuscate.cpp \
    ../src/Breakpoint.cpp \
    ../src/Globals.cpp \
    ../src/commands/CommandBreakpoint.cpp \
    ../src/SymbolTable.cpp \
    ../src/Symbol.cpp \
    ../src/commands/CommandStack.cpp \
    ../src/commands/CommandMemory.cpp \
    ../src/Tracee.cpp

HEADERS += \
    ../src/DebugLoop.hpp \
    ../src/commands/Command.hpp \
    ../src/commands/CommandContinue.hpp \
    ../src/commands/CommandDetach.hpp \
    ../src/commands/CommandExit.hpp \
    ../src/commands/CommandStep.hpp \
    ../src/commands/CommandRegisters.hpp \
    ../src/commands/CommandTracer.hpp \
    ../src/Globals.hpp \
    ../src/Mnemonic.hpp \
    ../src/Tracer.hpp \
    ../src/Globals.hpp \
    ../src/commands/CommandDisassemble.hpp \
    ../src/commands/CommandObfuscate.hpp \
    ../src/Breakpoint.hpp \
    ../src/commands/CommandBreakpoint.hpp \
    ../src/SymbolTable.hpp \
    ../src/Symbol.hpp \
    ../src/commands/CommandStack.hpp \
    ../src/commands/CommandMemory.hpp \
    ../src/Tracee.hpp

INCLUDEPATH += ../src
LIBS += -lbfd -ldl -liberty -lopcodes -lz
