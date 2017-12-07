#include "fbx.h"

#include "model.h"

#include <fbxsdk.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

extern bool all_formats;

bool CreateScene(model &m, const std::string &name, FbxManager* pSdkManager, FbxScene* pScene);

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if (!pManager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "myScene");
    if (!pScene)
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if (pManager)
        pManager->Destroy();
    if (!pExitStatus)
        FBXSDK_printf("Error: Unable to destroy FBX Manager!\n");
}

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, bool blender = false)
{
    int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pManager, "");

    auto pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat(); // 1 for ascii
    FbxString lDesc = pManager->GetIOPluginRegistry()->GetWriterFormatDescription(pFileFormat);

    // Set the export states. By default, the export states are always set to
    // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below
    // shows how to change these states.
    /*IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
    IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
    IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
    IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
    IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
    IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
    IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);*/

    // Initialize the exporter by providing a filename.
    if (lExporter->Initialize(pFilename, pFileFormat, pManager->GetIOSettings()) == false)
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        return false;
    }

    // Export the scene.
    // this one for blender
    if (blender)
        lExporter->SetFileExportVersion(FbxString("FBX201400"), FbxSceneRenamer::eNone);
    lStatus = lExporter->Export(pScene);

    // Destroy the exporter.
    lExporter->Destroy();
    return lStatus;
}

bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor, lSDKMinor, lSDKRevision;
    //int lFileFormat = -1;
    int i, lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pManager, "");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if (!lImportStatus)
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for (i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported.
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);

    if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
    {
        FBXSDK_printf("Please enter password: ");

        lPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(pScene);

        if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

void model::printFbx(const std::string &fn)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    CreateScene(*this, fn, lSdkManager, lScene);

    SaveScene(lSdkManager, lScene, (fn + "_ue4.fbx").c_str());
    if (all_formats)
        SaveScene(lSdkManager, lScene, (fn + "_blender.fbx").c_str(), true);

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, true);
}

bool CreateScene(model &model, const std::string &name, FbxManager* pSdkManager, FbxScene* pScene)
{
    static const char* gDiffuseElementName = "DiffuseUV";
    static const char* gAmbientElementName = "AmbientUV";
    static const char* gSpecularElementName = "SpecularUV";

    //std::map<std::string,
    int socket_id = 0;
    for (auto &b : model.blocks)
    {
        if (!b.canPrint())
            continue;

        //
        if (b.isEngineFx())
        {
            FbxNode *socket = FbxNode::Create(pScene, std::string("SOCKET_EngineFx_" + std::to_string(socket_id++)).c_str());
            auto n = FbxNull::Create(socket, "");
            socket->SetNodeAttribute(n);
            pScene->GetRootNode()->AddChild(socket);

            FbxVector4 c;
            for (auto &v : b.vertices)
            {
                FbxVector4 x;
                x.Set(-v.coordinates.x * scale_mult, v.coordinates.y * scale_mult, -v.coordinates.z * scale_mult, v.coordinates.w);
                c += x;
            }
            c /= b.vertices.size();

            socket->LclTranslation.Set(c);

            continue;
        }

        //auto block_name = name + "/" + b.h.name;
        const auto block_name = b.h.name;

        // mesh
        auto m = FbxMesh::Create(pSdkManager, block_name.c_str());

        // node
        FbxNode *node = FbxNode::Create(pScene, block_name.c_str());
        node->SetNodeAttribute(m);
        node->SetShadingMode(FbxNode::eTextureShading); // change?! was texture sh

        // vertices
        m->InitControlPoints(b.vertices.size());

        // normals
        auto normal = m->CreateElementNormal();
        normal->SetMappingMode(FbxGeometryElement::eByControlPoint);
        normal->SetReferenceMode(FbxGeometryElement::eDirect);

        // uv
        auto create_uv = [&m](auto &name)
        {
            auto uv1 = m->CreateElementUV(name);
            uv1->SetMappingMode(FbxGeometryElement::eByControlPoint);
            uv1->SetReferenceMode(FbxGeometryElement::eDirect);
            return uv1;
        };

        auto d_uv = create_uv(gDiffuseElementName);
        auto a_uv = create_uv(gAmbientElementName);
        auto s_uv = create_uv(gSpecularElementName);

        // add vertices, normals, uvs
        int i = 0;
        for (auto &v : b.vertices)
        {
            FbxVector4 x;
            x.Set(-v.coordinates.x * scale_mult, v.coordinates.z * scale_mult, v.coordinates.y * scale_mult, v.coordinates.w);
            m->SetControlPointAt(x, i++);
            normal->GetDirectArray().Add(FbxVector4(-v.normal.x, -v.normal.z, v.normal.y));

            float f;
            auto uc = modf(fabs(v.texture_coordinates.u), &f);
            auto vc = modf(fabs(v.texture_coordinates.v), &f);
            d_uv->GetDirectArray().Add(FbxVector2(uc, 1 - vc));
            a_uv->GetDirectArray().Add(FbxVector2(uc, 1 - vc));
            s_uv->GetDirectArray().Add(FbxVector2(uc, 1 - vc));
        }

        // faces
        for (auto &v : b.faces)
        {
            // Set the control point indices of the bottom side of the pyramid
            m->BeginPolygon();
            m->AddPolygon(v.x);
            m->AddPolygon(v.z);
            m->AddPolygon(v.y);
            m->EndPolygon();
        }

        // mats
        auto lMaterial = node->GetSrcObject<FbxSurfacePhong>(0);
        if (!lMaterial)
        {
            FbxString lMaterialName = block_name.c_str();
            FbxString lShadingName = "Phong";
            FbxDouble3 lAmbientColor(b.mat.ambient.r, b.mat.ambient.g, b.mat.ambient.b);
            FbxDouble3 lSpecularColor(b.mat.specular.r, b.mat.specular.g, b.mat.specular.b);
            FbxDouble3 lDiffuseColor(b.mat.diffuse.r, b.mat.diffuse.g, b.mat.diffuse.b);
            FbxDouble3 lEmissiveColor(b.mat.emissive.r, b.mat.emissive.g, b.mat.emissive.b);

            FbxLayer* lLayer = m->GetLayer(0);

            // Create a layer element material to handle proper mapping.
            FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(m, lMaterialName.Buffer());

            // This allows us to control where the materials are mapped.  Using eAllSame
            // means that all faces/polygons of the mesh will be assigned the same material.
            lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
            lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eDirect);

            // Save the material on the layer
            lLayer->SetMaterials(lLayerElementMaterial);

            // Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
            // we only need to add one.
            lLayerElementMaterial->GetIndexArray().Add(0);

            lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

            // Generate primary and secondary colors.
            lMaterial->Emissive.Set(lEmissiveColor);
            lMaterial->EmissiveFactor.Set(b.mat.emissive.r);

            lMaterial->Ambient.Set(lAmbientColor);
            lMaterial->AmbientFactor.Set(b.mat.ambient.r);

            // Add texture for diffuse channel
            lMaterial->Diffuse.Set(lDiffuseColor);
            lMaterial->DiffuseFactor.Set(b.mat.diffuse.r);

            //lMaterial->TransparencyFactor.Set(0.4);
            //lMaterial->ShadingModel.Set(lShadingName);
            //lMaterial->Shininess.Set(0.5);

            lMaterial->Specular.Set(lSpecularColor);
            lMaterial->SpecularFactor.Set(b.mat.power);
            node->AddMaterial(lMaterial);
        }

        if (b.mat_type != MaterialType::MaterialOnly)
        {
            FbxFileTexture* lTexture;

            // Set texture properties.
            lTexture = FbxFileTexture::Create(pScene, "Diffuse Texture");
            lTexture->SetFileName((b.h.tex_mask + texture_extension).c_str()); // Resource file is in current directory.
            lTexture->SetTextureUse(FbxTexture::eStandard);
            lTexture->SetMappingType(FbxTexture::eUV);
            lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
            lTexture->UVSet.Set(gDiffuseElementName);
            if (lMaterial)
                lMaterial->Diffuse.ConnectSrcObject(lTexture);

            // Set texture properties.
            lTexture = FbxFileTexture::Create(pScene, "Ambient Texture");
            lTexture->SetFileName((b.h.tex_mask + texture_extension).c_str()); // Resource file is in current directory.
            lTexture->SetTextureUse(FbxTexture::eStandard);
            lTexture->SetMappingType(FbxTexture::eUV);
            lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
            lTexture->UVSet.Set(gAmbientElementName);
            if (lMaterial)
                lMaterial->Ambient.ConnectSrcObject(lTexture);

            // Set texture properties.
            lTexture = FbxFileTexture::Create(pScene, "Specular Texture");
            lTexture->SetFileName((b.h.tex_spec + texture_extension).c_str()); // Resource file is in current directory.
            lTexture->SetTextureUse(FbxTexture::eStandard);
            lTexture->SetMappingType(FbxTexture::eUV);
            lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
            lTexture->UVSet.Set(gSpecularElementName);
            if (lMaterial)
                lMaterial->Specular.ConnectSrcObject(lTexture);
        }

        // add smoothing groups
        FbxGeometryConverter lGeometryConverter(pSdkManager);
        lGeometryConverter.ComputeEdgeSmoothingFromNormals(m);
        //convert soft/hard edge info to smoothing group info
        lGeometryConverter.ComputePolygonSmoothingFromEdgeSmoothing(m);

        //
        pScene->GetRootNode()->AddChild(node);
    }
    return true;
}
