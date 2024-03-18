pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

GenPackets.exe --path=./Protocol.proto --output=s2c_PacketHandler --recv=s2c_ --send=c2s_
IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../inc/Protocols"
XCOPY /Y Enum.pb.cc "../inc/Protocols"
XCOPY /Y Struct.pb.h "../inc/Protocols"
XCOPY /Y Struct.pb.cc "../inc/Protocols"
XCOPY /Y Protocol.pb.h "../inc/Protocols"
XCOPY /Y Protocol.pb.cc "../inc/Protocols"
XCOPY /Y s2c_PacketHandler.h "../inc/Protocols"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE