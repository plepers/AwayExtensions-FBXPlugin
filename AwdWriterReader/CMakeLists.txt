
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
  TextureExporter.cxx
  SkeletonExporter.cxx
  ExportContext.cxx
  utils.cxx
  tootle.cxx
  Timer.cxx

  AwdReader.h
  AwdWriter.h
  AwdWriterReader.h
  Settings.h
  BlocksMap.h
  NodeExporter.h
  ContainerExporter.h
  MeshExporter.h
  GeomExporter.h
  GeomData.h
  MaterialExporter.h
  SkeletonExporter.h
  TextureExporter.h
  ExportContext.h
  utils.h
  tootle.h
  Timer.h
)

add_library( AwdWriterReader_static ${AwdWriterReader_SOURCES} )

add_library( AwdWriterReader SHARED ${AwdWriterReader_SOURCES} )
TARGET_LINK_LIBRARIES ( AwdWriterReader ${LINK_LIBRARIES} )

if( FBX_EXT_FOUND )

  add_library( AwdWriterReader_max SHARED ${AwdWriterReader_SOURCES} )
  TARGET_LINK_LIBRARIES ( AwdWriterReader_max ${FBX_EXT_LIBRARIES} ${AWD_LIBRARIES} )

endif()