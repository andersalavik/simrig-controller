Import("env")

print(env.Dump())

board_config = env.BoardConfig()
# should be array of VID:PID pairs
#
# USB VID:PID=2341:8037 SER=HIDPB

board_config.update("build.hwids", [
  ["0x2341", "0x8037"]
])