/*=========================================================================

  Program:   ParaView
  Module:    vtkPVWorldPointPicker.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkPVWorldPointPicker.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkPVRenderModule.h"
#include "vtkRenderer.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVWorldPointPicker);
vtkCxxRevisionMacro(vtkPVWorldPointPicker, "1.8");

vtkCxxSetObjectMacro(vtkPVWorldPointPicker, RenderModule, vtkPVRenderModule);

//----------------------------------------------------------------------------
vtkPVWorldPointPicker::vtkPVWorldPointPicker()
{
  this->RenderModule = NULL;
}

//----------------------------------------------------------------------------
vtkPVWorldPointPicker::~vtkPVWorldPointPicker()
{
  this->SetRenderModule(NULL);
}


// Perform pick operation with selection point provided. The z location
// is recovered from the zBuffer. Always returns 0 since no actors are picked.
int vtkPVWorldPointPicker::Pick(float selectionX, float selectionY, 
                              float selectionZ, vtkRenderer *renderer)
{
  vtkCamera *camera;
  double cameraFP[4];
  double display[3], *world;
  double *displayCoord;
  double z;

  if (this->RenderModule == NULL)
    {
    return vtkWorldPointPicker::Pick(selectionX, selectionY, selectionZ, renderer);
    }

  // Initialize the picking process
  this->Initialize();
  this->Renderer = renderer;
  this->SelectionPoint[0] = selectionX;
  this->SelectionPoint[1] = selectionY;
  this->SelectionPoint[2] = selectionZ;

  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent,NULL);

  z = this->RenderModule->GetZBufferValue ((int) selectionX, (int) selectionY);
  
  // if z is 1.0, we assume the user has picked a point on the
  // screen that has not been rendered into. Use the camera's focal
  // point for the z value. The test value .999999 has to be used
  // instead of 1.0 because for some reason our SGI Infinite Reality
  // engine won't return a 1.0 from the zbuffer
  if (z < 0.999999)
    {
    selectionZ = z;
    vtkDebugMacro(<< " z from zBuffer: " << selectionZ);
    }
  else
    {
    // Get camera focal point and position. Convert to display (screen) 
    // coordinates. We need a depth value for z-buffer.
    camera = renderer->GetActiveCamera();
    camera->GetFocalPoint(cameraFP); cameraFP[3] = 1.0;

    renderer->SetWorldPoint(cameraFP);
    renderer->WorldToDisplay();
    displayCoord = renderer->GetDisplayPoint();
    selectionZ = displayCoord[2];
    vtkDebugMacro(<< "computed z from focal point: " << selectionZ);
    }

  // now convert the display point to world coordinates
  display[0] = selectionX;
  display[1] = selectionY;
  display[2] = selectionZ;

  renderer->SetDisplayPoint (display);
  renderer->DisplayToWorld ();
  world = renderer->GetWorldPoint ();
  
  for (int i=0; i < 3; i++) 
    {
    this->PickPosition[i] = world[i] / world[3];
    }

  // Invoke end pick method if defined
  this->InvokeEvent(vtkCommand::EndPickEvent,NULL);

  return 0;
}



//----------------------------------------------------------------------------
void vtkPVWorldPointPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkWorldPointPicker::PrintSelf(os,indent);

  os << indent << "RenderModule: " << this->RenderModule << endl;
}

