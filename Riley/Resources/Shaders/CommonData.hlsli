#ifndef _COMMON_DATA_
#define _COMMON_DATA_

struct FrameBufferConsts
{
    Matrix viewRow;
    Matrix projRow;
    Matrix viewProjRow;
    
    Matrix invViewRow;
    Matrix invProjRow;
    Matrix invViewProjRow;

    Matrix prevViewRow;
    Matrix prevProjRow;
    Matrix prevViewProjRow;

    float4 globalAmbient;
    float4 cameraPosition;
    float4 cameraFoward;

    float cameraNear;
    float cameraFar;
    float cameraJitterX;
    float cameraJitterY;

    float screenResolutionX;
    float screenResolutionY;
    float mouseNormalizedCoordsX;
    float mouseNormalizedCoordsY;
};

#endif