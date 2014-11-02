
#include <fbxsdk.h>
#include "AwdWriterReader.h"

class AwdWriterReaderPlugin : public FbxPlugin
{
    FBXSDK_PLUGIN_DECLARE(AwdWriterReaderPlugin);

protected:
    explicit AwdWriterReaderPlugin(const FbxPluginDef& pDefinition, FbxModule pLibHandle) : FbxPlugin(pDefinition, pLibHandle)
    {
    }

    // Implement Fbxmodules::FbxPlugin
    virtual bool SpecificInitialize()
    {
        int FirstPluginID, RegistredCount;
        GetData().mSDKManager->GetIOPluginRegistry()->RegisterReader(CreateAwdReader, GetAwdReaderInfo, FirstPluginID, RegistredCount, FillOwnReaderIOSettings);
        GetData().mSDKManager->GetIOPluginRegistry()->RegisterWriter(CreateAwdWriter, GetAwdWriterInfo, FirstPluginID, RegistredCount, FillOwnWriterIOSettings);
        return true;
    }

    virtual bool SpecificTerminate()
    {
        return true;
    }
};

 FBXSDK_PLUGIN_IMPLEMENT(AwdWriterReaderPlugin); //This macro is mandatory for any plug-in implementation

 //Standard C export needed for any new FBX SDK plug-in
 extern "C"
 {
     static AwdWriterReaderPlugin* sPluginInstance = NULL; //The module is owner of the plug-in

     //This function will be called when an application will request the plug-in
 #ifdef FBXSDK_ENV_WIN
     __declspec(dllexport) void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pModuleHandle)
 #else
     void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pModuleHandle)
 #endif
     {
         if( sPluginInstance == NULL )
         {
             //Create the plug-in definition which contains the information about the plug-in
             FbxPluginDef sPluginDef;
             sPluginDef.mName = "AWD plugin";
             sPluginDef.mVersion = "1.0";

             //Create an instance of the plug-in
             sPluginInstance = AwdWriterReaderPlugin::Create(sPluginDef, pModuleHandle);

             //Register the plug-in with the FBX SDK
             pContainer.Register(*sPluginInstance);
         }
     }
 }