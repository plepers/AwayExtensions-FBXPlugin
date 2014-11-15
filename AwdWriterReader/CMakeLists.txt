
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
  ExportContext.cxx

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
  ExportContext.h
)

add_library( AwdWriterReader_static ${AwdWriterReader_SOURCES} )

add_library( AwdWriterReader SHARED ${AwdWriterReader_SOURCES} )
TARGET_LINK_LIBRARIES ( AwdWriterReader ${LINK_LIBRARIES} )

if( FBX_EXT_FOUND )

  add_library( AwdWriterReader_max SHARED ${AwdWriterReader_SOURCES} )
  TARGET_LINK_LIBRARIES ( AwdWriterReader_max ${FBX_EXT_LIBRARIES} ${AWD_LIBRARIES} )

endif()