pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

GenPackets.exe --path=./Protocol.proto --output=c2s_PacketHandler --recv=c2s_ --send=s2c_
IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../Libraries/Protocols"
XCOPY /Y Enum.pb.cc "../../Libraries/Protocols"
XCOPY /Y Struct.pb.h "../../Libraries/Protocols"
XCOPY /Y Struct.pb.cc "../../Libraries/Protocols"
XCOPY /Y Protocol.pb.h "../../Libraries/Protocols"
XCOPY /Y Protocol.pb.cc "../../Libraries/Protocols"
XCOPY /Y c2s_PacketHandler.h "../../Libraries/Protocols"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE