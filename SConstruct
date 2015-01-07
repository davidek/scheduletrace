
DEBUG = True

src_files = Glob('src/**.c')

env = Environment()
#env.Append(CCFLAGS=['-ansi'])  # currently not compatible
env.Append(CCFLAGS=['-Wall', '-Wpedantic'])
env.Append(CCFLAGS=['-pthread'], LINKFLAGS=['-pthread'])
env.Append(LIBS=['rt'])
env.ParseConfig('allegro-config --libs')

if DEBUG:
    env.Append(CCFLAGS=['-g'])
else:
    env.Append(CCFLAGS=['-DNDEBUG', '-O3'])

env.Program('scheduletrace', src_files)


# vim: set filetype=python:
