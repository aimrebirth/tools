#include "model.h"

#include <boost/algorithm/string.hpp>
#include <primitives/string.h>
#include <primitives/templates.h>
#include <primitives/sw/cl.h>
#include <fbxsdk.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

cl::opt<bool> text_fbx("t", cl::desc("Produce ascii .fbx"));
extern cl::opt<bool> all_formats;

bool CreateScene(const model &m, const std::string &name, FbxManager* pSdkManager, FbxScene* pScene);

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

    pScene->GetSceneInfo()->LastSaved_ApplicationVendor.Set("lzwdgc's");
    pScene->GetSceneInfo()->LastSaved_ApplicationName.Set("mod_converter");
    pScene->GetSceneInfo()->LastSaved_ApplicationVersion.Set(version().c_str());
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
    //int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pManager, "");

    auto pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();
    if (text_fbx)
        pFileFormat = 1; // 1 for ascii
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

void ConvertScene(FbxScene* lScene, AxisSystem as)
{
    switch (as)
    {
    case AxisSystem::eMayaYUp:
        // "By default the FbxScene uses a Y-Up axis system."
        // https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_cpp_ref_class_fbx_axis_system_html
        break;
    case AxisSystem::eMayaZUp:
        FbxAxisSystem::MayaZUp.ConvertScene(lScene);
        break;
    case AxisSystem::eDirectX:
        FbxAxisSystem::DirectX.ConvertScene(lScene);
        break;
    case AxisSystem::eWindows3DViewer:
        // UpVector = +Z, FrontVector = +Y, CoordSystem = +X (RightHanded)
        // UpVector = ZAxis, FrontVector = ParityOdd, CoordSystem = RightHanded
        FbxAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded).ConvertScene(lScene);
        break;
    default:
        SW_UNIMPLEMENTED;
    }
}

void model::printFbx(const std::string &fn, AxisSystem as) const
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    CreateScene(*this, fn, lSdkManager, lScene);

    ConvertScene(lScene, as);
    //FbxSystemUnit::cm.ConvertScene(lScene);

    SaveScene(lSdkManager, lScene, (fn + ".fbx").c_str());
    //SaveScene(lSdkManager, lScene, (fn + "_ue4.fbx").c_str());
    //if (all_formats)
        //SaveScene(lSdkManager, lScene, (fn + "_blender.fbx").c_str(), true);

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, true);
}

static FbxMesh *create_mesh(FbxScene *s, const block &b)
{
    auto m = FbxMesh::Create(s, b.h.name.c_str());

    // vertexes
    {
        m->InitControlPoints(b.pmd.vertices.size());
        auto cps = m->GetControlPoints();
        for (const auto &[i, v] : enumerate(b.pmd.vertices))
            cps[i] = FbxVector4(v.x, v.y, v.z);
    }

    // normals
    {
        auto normal = m->CreateElementNormal();
        normal->SetMappingMode(FbxGeometryElement::eByControlPoint);
        normal->SetReferenceMode(FbxGeometryElement::eDirect);
        for (const auto &v : b.pmd.normals)
            normal->GetDirectArray().Add(FbxVector4(v.x, v.y, v.z));
    }

    // uvs
    {
        auto uv = m->CreateElementUV("uv", FbxLayerElement::eTextureDiffuse);
        uv->SetMappingMode(FbxGeometryElement::eByControlPoint);
        //uv->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
        uv->SetReferenceMode(FbxGeometryElement::eDirect); // not needed?
        for (const auto &[u,v] : b.pmd.uvs)
            uv->GetDirectArray().Add(FbxVector2(u, v));
        //float f;
        //auto uc = modf(fabs(u), &f);
        //auto vc = modf(fabs(v), &f);
    }

    // faces
    for (auto &v : b.pmd.faces)
    {
        // Set the control point indices of the bottom side of the pyramid
        m->BeginPolygon();
        for (auto &i : v.points)
            m->AddPolygon(i.vertex/*, i.uv*/);
        m->EndPolygon();
    }

    // add smoothing groups
    {
        FbxGeometryConverter lGeometryConverter(m->GetFbxManager());
        lGeometryConverter.ComputeEdgeSmoothingFromNormals(m);
        //convert soft/hard edge info to smoothing group info
        lGeometryConverter.ComputePolygonSmoothingFromEdgeSmoothing(m);
    }

    return m;
}

static void set_material(FbxMesh *m, const block &b)
{
    m->GetNode()->RemoveAllMaterials();

    auto mat = m->CreateElementMaterial();
    // This allows us to control where the materials are mapped.  Using eAllSame
    // means that all faces/polygons of the mesh will be assigned the same material.
    mat->SetMappingMode(FbxLayerElement::eAllSame);
    mat->SetReferenceMode(FbxLayerElement::eDirect);

    auto lMaterial = FbxSurfacePhong::Create(m->GetNode(), b.h.name.c_str());

    FbxDouble3 lAmbientColor(b.mat.ambient.r, b.mat.ambient.g, b.mat.ambient.b);
    FbxDouble3 lSpecularColor(b.mat.specular.r, b.mat.specular.g, b.mat.specular.b);
    FbxDouble3 lDiffuseColor(b.mat.diffuse.r, b.mat.diffuse.g, b.mat.diffuse.b);
    FbxDouble3 lEmissiveColor(b.mat.emissive.r, b.mat.emissive.g, b.mat.emissive.b);

    // Generate primary and secondary colors.
    lMaterial->Emissive.Set(lEmissiveColor);
    lMaterial->EmissiveFactor.Set(b.mat.emissive.r);

    lMaterial->Ambient.Set(lAmbientColor);
    lMaterial->AmbientFactor.Set(b.mat.ambient.r);

    lMaterial->Diffuse.Set(lDiffuseColor);
    lMaterial->DiffuseFactor.Set(b.mat.diffuse.r);

    //lMaterial->TransparencyFactor.Set(0.4);
    lMaterial->ShadingModel.Set("Phong");
    //lMaterial->Shininess.Set(0.5);

    lMaterial->Specular.Set(lSpecularColor);
    lMaterial->SpecularFactor.Set(b.mat.power);

    m->GetNode()->AddMaterial(lMaterial);
}

static void set_textures(FbxMesh *m, const block &b)
{
    auto add_tex = [m, &b](String name, String texname, auto &what)
    {
        auto lTexture = FbxFileTexture::Create(m->GetNode(), (name + " Texture").c_str());
        lTexture->SetFileName((texname + texture_extension).c_str()); // Resource file is in current directory.
        lTexture->SetTextureUse(FbxTexture::eStandard);
        lTexture->SetMappingType(FbxTexture::eUV);
        lTexture->SetMaterialUse(FbxFileTexture::eDefaultMaterial);
        lTexture->UVSet.Set(m->GetElementUV()->GetName());

        what.ConnectSrcObject(lTexture);
    };

    auto mat = (FbxSurfacePhong *)m->GetNode()->GetMaterial(0);
    add_tex("Specular", b.h.spec.name, mat->Specular);
    add_tex("Diffuse", b.h.mask.name, mat->Diffuse);
    add_tex("Ambient", b.h.mask.name, mat->Ambient);
}

bool CreateScene(const model &model, const std::string &name, FbxManager* pSdkManager, FbxScene* pScene)
{
    static const char* gDiffuseElementName = "DiffuseUV";
    static const char* gAmbientElementName = "AmbientUV";
    static const char* gSpecularElementName = "SpecularUV";

    int engine_id = 0;
    int fx_id = 0;
    for (auto &b : model.blocks)
    {
        auto create_socket_named = [&pScene](const std::string &name)
        {
            FbxNode *socket = FbxNode::Create(pScene, ("SOCKET_" + name).c_str());
            auto n = FbxNull::Create(pScene, "");
            socket->SetNodeAttribute(n);
            pScene->GetRootNode()->AddChild(socket);
            return socket;
        };

        auto create_socket = [&create_socket_named](const auto &b, const std::string &name, bool mirror_y = false)
        {
            FbxVector4 c;
            for (auto &v : b.pmd.vertices)
            {
                FbxVector4 x;
                x.Set(v.x * scale_mult(), v.y * scale_mult(), v.z * scale_mult(), v.w);
                c += x;
            }
            c /= b.pmd.vertices.size();

            auto s = create_socket_named(name);
            if (mirror_y) // y is the second coord, idx = 1
                c.mData[1] = -c.mData[1];
            s->LclTranslation.Set(c);
        };

        //
        if (b.isEngineFx())
        {
            create_socket(b, "EngineFx_" + std::to_string(engine_id++));
            continue;
        }
        else if (b.h.name == boost::to_lower_copy("LIGHTGUN"s))
        {
            create_socket(b, "WeaponLight_0");
            create_socket(b, "WeaponLight_1", true);
            continue;
        }
        else if (b.h.name == boost::to_lower_copy("HEAVYGUN"s))
        {
            create_socket(b, "WeaponHeavy");
            continue;
        }
        else if (b.h.name == boost::to_lower_copy("ROCKET"s))
        {
            create_socket(b, "WeaponRocket");
            continue;
        }
        else if (b.h.name.find(boost::to_lower_copy("FX"s)) == 0)
        {
            create_socket(b, "Fx_" + std::to_string(fx_id++));
            continue;
        }

        if (!b.canPrint())
            continue;

        //auto block_name = name + "/" + b.h.name;
        const auto block_name = b.h.name;

        // mesh node
        FbxNode *node = FbxNode::Create(pScene, block_name.c_str());
        pScene->GetRootNode()->AddChild(node);
        auto m = create_mesh(pScene, b);
        node->SetNodeAttribute(m);
        set_material(m, b);
        if (b.mat_type != MaterialType::MaterialOnly)
            set_textures(m, b);

        //node->SetShadingMode(FbxNode::eFullShading); // change?! was texture sh

        // scale
        node->LclScaling.Set(FbxDouble3(scale_mult(), scale_mult(), scale_mult()));

        //m->BuildMeshEdgeArray();
    }

    return true;
}
