import subprocess

Import("env")

# # Dump global construction environment (for debug purpose)
# print(env.Dump())

# Dump project construction environment (for debug purpose)
# print(projenv.Dump())

describe = subprocess.run(['git', 'describe', "--match=v[0-9]*.[0-9]*", "--dirty"], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
flavour = env["PIOENV"]
fwname = f'{flavour}-{describe}'

env.Replace(PROGNAME=f'firmware-{fwname}')

lines = [
    f'const char *FIRMWARE_VERSION = "{describe}";\n',
    f'const char *FIRMWARE_FLAVOUR = "{flavour}";\n'
]

f = open("src/version.cpp", "w")
f.writelines(lines)
f.close
