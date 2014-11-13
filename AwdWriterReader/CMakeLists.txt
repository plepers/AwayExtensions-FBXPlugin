
set( AwdWriterReader_SOURCES
  AwdReader.cxx
  AwdWriter.cxx
  AwdWriterReader.cxx
  AwdPlugin.cxx
  Settings.cxx
  BlocksMap.cxx
  NodeExporter.cxx
  ContainerExporter.cxx
  MeshExporter.cxx
  GeomExporter.cxx
  MaterialExporter.cxx
  
  AwdReader.h
  AwdWriter.h
  AwdWriterReader.h
  Settings.h
  BlocksMap.h
  NodeExporter.h
  ContainerExporter.h
  MeshExporter.h
  GeomExporter.h
  MaterialExporter.h
)

add_library( AwdWriterReader_static ${AwdWriterReader_SOURCES} )

add_library( AwdWriterReader SHARED ${AwdWriterReader_SOURCES} )
TARGET_LINK_LIBRARIES ( AwdWriterReader ${LINK_LIBRARIES} )