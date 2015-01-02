
DEBUG = True

src_files = Glob('src/**.c')

env = Environment()
env.ParseConfig('allegro-config --libs')
if DEBUG:
    env.Append(CCFLAGS=['-g', '-Wall'])


env.Program('scheduletrace', src_files)

