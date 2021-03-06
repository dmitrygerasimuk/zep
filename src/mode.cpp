#include "buffer.h"
#include "mode.h"
#include "editor.h"
#include "commands.h"

namespace Zep
{

ZepMode::ZepMode(ZepEditor& editor)
    : ZepComponent(editor),
    m_currentMode(EditorMode::Normal)
{
}

ZepMode::~ZepMode()
{

}

void ZepMode::SetCurrentWindow(ZepWindow* pDisplay)
{
    m_pCurrentWindow = pDisplay;
}

void ZepMode::AddCommandText(std::string strText)
{
    for (auto& ch : strText)
    {
        AddKeyPress(ch);
    }
}

void ZepMode::AddCommand(std::shared_ptr<ZepCommand> spCmd)
{
    spCmd->Redo();
    m_undoStack.push(spCmd);

    // Can't redo anything beyond this point
    std::stack<std::shared_ptr<ZepCommand>> empty;
    m_redoStack.swap(empty);
}

void ZepMode::Redo()
{
    bool inGroup = false;
    do 
    {
        if (!m_redoStack.empty())
        {
            auto& spCommand = m_redoStack.top();
            spCommand->Redo();

            if (spCommand->GetFlags() & CommandFlags::GroupBoundary)
            {
                inGroup = !inGroup;
            }

            m_undoStack.push(spCommand);
            m_redoStack.pop();
        }
        else
        {
            break;
        }
    } while (inGroup);
}

void ZepMode::Undo()
{
    bool inGroup = false;
    do
    {
        if (!m_undoStack.empty())
        {
            auto& spCommand = m_undoStack.top();
            spCommand->Undo();

            if (spCommand->GetFlags() & CommandFlags::GroupBoundary)
            {
                inGroup = !inGroup;
            }

            m_redoStack.push(spCommand);
            m_undoStack.pop();
        }
        else
        {
            break;
        }
    } 
    while (inGroup);
}

void ZepMode::UpdateVisualSelection()
{
    // Visual mode update - after a command
    if (m_currentMode == EditorMode::Visual)
    {
        // Update the visual range
        if (m_lineWise)
        {
            auto pLineInfo = &m_pCurrentWindow->visibleLines[m_pCurrentWindow->GetCursor().y];
            m_visualEnd = m_pCurrentWindow->GetCurrentBuffer()->GetLinePos(pLineInfo->lineNumber, LineLocation::LineEnd) - 1;
        }
        else
        {
            m_visualEnd = m_pCurrentWindow->DisplayToBuffer();
        }
        m_pCurrentWindow->SetSelectionRange(m_pCurrentWindow->BufferToDisplay(m_visualBegin), m_pCurrentWindow->BufferToDisplay(m_visualEnd));
    }
}
}

