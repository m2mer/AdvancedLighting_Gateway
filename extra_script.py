Import("env")
import time

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}

buildtime = time.strftime('%Y%m%d%H%M%S', time.localtime(time.time()))

print buildtime

#env.Replace(PROGNAME="firmware_%s" % buildtime)
env.Replace(PROGNAME = "firmware_%s" % defines.get("VERSION"))
