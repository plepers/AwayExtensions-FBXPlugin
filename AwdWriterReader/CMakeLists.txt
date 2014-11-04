
set( AwdWriterReader_SOURCES
  AwdReader.cxx
  AwdWriter.cxx
  AwdWriterReader.cxx
  AwdPlugin.cxx
  Settings.cxx
  ContainerExporter.cxx
  NodeExporter.cxx
  
  AwdReader.h
  AwdWriter.h
  AwdWriterReader.h
  Settings.h
  ContainerExporter.h
  NodeExporter.h
)

add_library( AwdWriterReader_static ${AwdWriterReader_SOURCES} )

add_library( AwdWriterReader SHARED ${AwdWriterReader_SOURCES} )
TARGET_LINK_LIBRARIES ( AwdWriterReader ${LINK_LIBRARIES} )