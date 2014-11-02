
set( AwdWriterReader_SOURCES
  AwdReader.cxx
  AwdWriter.cxx
  AwdWriterReader.cxx
  AwdPlugin.cxx
  AwdReader.h
  AwdWriter.h
  AwdWriterReader.h
)

add_library(AwdWriterReader ${AwdWriterReader_SOURCES})