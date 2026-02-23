// cmdBlackHole_RealTimeRender.cpp : command file
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderPlugIn.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// BEGIN BlackHole_RealTimeRender command
//

#pragma region BlackHole_RealTimeRender command

// Do NOT put the definition of class CCommandBlackHole_RealTimeRender in a header
// file. There is only ONE instance of a CCommandBlackHole_RealTimeRender class
// and that instance is the static theBlackHole_RealTimeRenderCommand that appears
// immediately below the class definition.

class CCommandBlackHole_RealTimeRender : public CRhinoCommand
{
public:
  // The one and only instance of CCommandBlackHole_RealTimeRender is created below.
  // No copy constructor or operator= is required.
  // Values of member variables persist for the duration of the application.

  // CCommandBlackHole_RealTimeRender::CCommandBlackHole_RealTimeRender()
  // is called exactly once when static theBlackHole_RealTimeRenderCommand is created.
  CCommandBlackHole_RealTimeRender() = default;

  // CCommandBlackHole_RealTimeRender::~CCommandBlackHole_RealTimeRender()
  // is called exactly once when static theBlackHole_RealTimeRenderCommand is destroyed.
  // The destructor should not make any calls to the Rhino SDK. 
  // If your command has persistent settings, then override 
  // CRhinoCommand::SaveProfile and CRhinoCommand::LoadProfile.
  ~CCommandBlackHole_RealTimeRender() = default;

  // Returns a unique UUID for this command.
  // If you try to use an id that is already being used, then
  // your command will not work. Use GUIDGEN.EXE to make unique UUID.
  UUID CommandUUID() override
  {
    // {051D79B6-8712-4EAF-9549-8DADC3E77C28}
    static const GUID BlackHole_RealTimeRenderCommand_UUID = 
    {0x051d79b6,0x8712,0x4eaf,{0x95,0x49,0x8d,0xad,0xc3,0xe7,0x7c,0x28}};
    return BlackHole_RealTimeRenderCommand_UUID;
  }

  // Returns the English command name.
  // If you want to provide a localized command name, then override 
  // CRhinoCommand::LocalCommandName.
  const wchar_t* EnglishCommandName() override { return L"BlackHole_RealTimeRender"; }

  // Rhino calls RunCommand to run the command.
  CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override;
};

// The one and only CCommandBlackHole_RealTimeRender object
// Do NOT create any other instance of a CCommandBlackHole_RealTimeRender class.
static class CCommandBlackHole_RealTimeRender theBlackHole_RealTimeRenderCommand;

CRhinoCommand::result CCommandBlackHole_RealTimeRender::RunCommand(const CRhinoCommandContext& context)
{
  // CCommandBlackHole_RealTimeRender::RunCommand() is called when the user
  // runs the "BlackHole_RealTimeRender".

  // TODO: Add command code here.

  // Rhino command that display a dialog box interface should also support
  // a command-line, or script-able interface.

  ON_wString str;
  str.Format(L"The \"%s\" command is under construction.\n", EnglishCommandName());
  const wchar_t* pszStr = static_cast<const wchar_t*>(str);
  if (context.IsInteractive())
    RhinoMessageBox(pszStr, BlackHole_RealTimeRenderPlugIn().PlugInName(), MB_OK);
  else
    RhinoApp().Print(pszStr);

  // TODO: Return one of the following values:
  //   CRhinoCommand::success:  The command worked.
  //   CRhinoCommand::failure:  The command failed because of invalid input, inability
  //                            to compute the desired result, or some other reason
  //                            computation reason.
  //   CRhinoCommand::cancel:   The user interactively canceled the command 
  //                            (by pressing ESCAPE, clicking a CANCEL button, etc.)
  //                            in a Get operation, dialog, time consuming computation, etc.

  return CRhinoCommand::success;
}

#pragma endregion

//
// END BlackHole_RealTimeRender command
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
