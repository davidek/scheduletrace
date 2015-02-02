
DEBUG = True

src_files = Glob('src/**.c')

env = Environment()
env.Append(CCFLAGS=['-std=c99', '-D_GNU_SOURCE'])
env.Append(CCFLAGS=['-Wall', '-Wpedantic'])
env.Append(CCFLAGS=['-pthread'], LINKFLAGS=['-pthread'])
env.Append(LIBS=['rt'])
env.ParseConfig('allegro-config --libs')

if DEBUG:
    env.Append(CCFLAGS=['-g', '-Werror'])
else:
    env.Append(CCFLAGS=['-DNDEBUG', '-O3'])

# print env.Dump()  # verbosely displays the building environment

executable = env.Program('scheduletrace', src_files)
NoClean(executable)


# vim: set filetype=python:
