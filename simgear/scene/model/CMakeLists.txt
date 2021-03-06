include (SimGearComponent)

set(HEADERS 
    BoundingVolumeBuildVisitor.hxx
    BVHDebugCollectVisitor.hxx
    BVHPageNodeOSG.hxx
    CheckSceneryVisitor.hxx
    ConditionNode.hxx
    ModelRegistry.hxx
    PrimitiveCollector.hxx
    SGClipGroup.hxx
    SGInteractionAnimation.hxx
    SGMaterialAnimation.hxx
    SGPickAnimation.hxx
    SGOffsetTransform.hxx
    SGReaderWriterXML.hxx
    SGRotateTransform.hxx
    SGScaleTransform.hxx
    SGText.hxx
    SGTranslateTransform.hxx
    animation.hxx
    model.hxx
    modellib.hxx
    particles.hxx
    persparam.hxx
    placement.hxx
    )

set(SOURCES 
    BVHPageNodeOSG.cxx
    CheckSceneryVisitor.cxx
    ConditionNode.cxx
    ModelRegistry.cxx
    PrimitiveCollector.cxx
    SGClipGroup.cxx
    SGInteractionAnimation.cxx
    SGLightAnimation.cxx
    SGPickAnimation.cxx
    SGMaterialAnimation.cxx
    SGOffsetTransform.cxx
    SGReaderWriterXML.cxx
    SGRotateTransform.cxx
    SGScaleTransform.cxx
    SGText.cxx
    SGTranslateTransform.cxx
    animation.cxx
    model.cxx
    modellib.cxx
    particles.cxx
    persparam.cxx
    placement.cxx    
    shadanim.cxx
    )

simgear_scene_component(model scene/model "${SOURCES}" "${HEADERS}")
