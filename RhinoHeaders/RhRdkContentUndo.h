
#pragma once // Public SDK header

#include "RhRdkDefinitions.h"

class CRhRdkContent;

class RHRDK_SDK IRhRdkContentUndo
{
public:
	/** This method is implemented as \code delete this; \endcode */
	virtual void DeleteThis(void) = 0;

	/** Call this after creating a new content but before attaching it to a document.
		\param c is the new content being added.
		\param pParent is a pointer to the content that will become the parent of the new content,
		 or null if the new content is being added at the top level (i.e., not a child).
		\return \e true if successful, else \e false. */
	virtual bool AddContent(const CRhRdkContent& c, const CRhRdkContent* pParent) = 0;

	/** Call this before modifying a content in a significant way, for example changing a child.
		\note If you are just changing a simple parameter such as color, you should call TweakContent() instead.
		\param c is the content being modified. */
	virtual bool ModifyContent(const CRhRdkContent& c) = 0;

	/** Call this before tweaking a single content parameter. It is more efficient than calling ModifyContent()
		because it only saves and restores the single changed parameter instead of the whole content hierarchy.
		\param c is the content being tweaked.
		\param wszParamName is the name of the parameter being tweaked.
		\param vValueOld is the value of the parameter before the tweak is applied. */
	virtual bool TweakContent(const CRhRdkContent& c, const wchar_t* wszParamName, const class CRhRdkVariant& vValueOld) = 0;

	/** Call this before 'deleting' one or more top-level contents. This requires the caller to
		merely detach the contents from the document and pass ownership of them to this object.
		It eliminates the need to copy the contents and the need to actually delete them, thus
		saving time and memory. Because the contents are not actually deleted at all, the user
		still perceives them as deleted because they disappear from the UI when detached.
		\param aContent is an array of pointers to the contents being 'deleted'.
		 If any one of these contents is a child, the function will fail. */
	virtual bool DeleteTopLevelContent(const CRhRdkContentArrayNC& aContent) = 0;

	/** Call this before deleting one or more contents. This copies the content hierarchies
		into the undo object and is therefore somewhat inefficient. If all the contents are
		top-level, it is better to call DeleteTopLevelContent() instead.
		\param aContent is an array of pointers to the contents being deleted. */
	virtual bool DeleteContent(const CRhRdkContentArray& aContent) = 0;

	/** Call this before promoting a single content from reference to non-reference.
		\param c is the content being promoted. */
	/*virtual bool PromoteContent(const CRhRdkContent& c) = 0;*/ // [SDK_UNFREEZE]

	/** Emergency virtual function for future expansion.
		"PromoteContent" is supported. */
	virtual void* EVF(const wchar_t*, void*) = 0;

protected:
	virtual ~IRhRdkContentUndo() { }
};

/** \class CRhRdkContentUndo

	This is a stack object that makes it easy for clients to implement content undo.
	If you use the CRhRdkContentField object for storing your content's variables, you may not
	need to use this class since the field system takes care of parameter undo automatically.
	Use this class for handling parameter undo for data that is not stored in fields, or for
	handling custom content adds and deletes.

	\see CRhRdkContentField

	\note For this to work, your UI is responsible for starting an undo record.

	\see CRhRdkUndoableEvent
	\see CRhRdkNewUndoableEvent
	\see uuidData_RdkUndo

*/
class RHRDK_SDK CRhRdkContentUndo : public IRhRdkContentUndo
{
public:
	/** This constructor is deprecated in favor of the one below.
		\param wszDesc is obsolete and not used. You must make sure
		 an undo record is active at the time this object is created.
		\see CRhRdkUndoableEvent \see uuidData_RdkUndo */
	RDK_DEPRECATED CRhRdkContentUndo(const wchar_t* wszDesc, INT_PTR reserved=0);

	/** Use this constructor if you have a reference to a Rhino document.
		\note You must make sure an undo record is active at the time this
		 object is created or it will have no effect.
		\see CRhRdkUndoableEvent \see uuidData_RdkUndo */
	CRhRdkContentUndo(const CRhinoDoc& doc);

	/** Use this constructor if you have a pointer to an RDK document.
		The pointer can be null. Undo will only be recorded if the RDK
		document is not null and is associated with a Rhino document.
		\note You must make sure an undo record is active at the time this
		 object is created or it will have no effect.
		\see CRhRdkUndoableEvent \see uuidData_RdkUndo */
	CRhRdkContentUndo(const class CRhRdkDocument* pRdkDoc);

	virtual ~CRhRdkContentUndo();

	/** Call this after creating a new content but before adding it to the document.
		\param newContent is the new content being added.
		\param pParent is a pointer to the content that will become the parent of the new content,
		 or null if the new content is being added at the top level (i.e., not a child).
		\return \e true if successful, else \e false. */
	virtual bool AddContent(const CRhRdkContent& newContent, const CRhRdkContent* pParent) override;

	/** \note This method is obsolete because it does not support redo. It has only been retained
		 for backward compatibility. Please call the other AddContent() method instead.*/
	RDK_DEPRECATED virtual bool AddContent(const CRhRdkContent& newContent);

	/** Call this before modifying a content in a significant way, for example changing a child.
		\note If you are just changing a simple parameter such as color, you should call TweakContent() instead.
		\param content is the content being modified. */
	virtual bool ModifyContent(const CRhRdkContent& content) override;

	/** Call this before tweaking a single content parameter. It is more efficient than calling ModifyContent()
		because it only saves and restores the single changed parameter instead of the whole content hierarchy.
		\param content is the content being tweaked.
		\param wszParamName is the name of the parameter being tweaked.
		\param vValueOld is the value of the parameter before the tweak is applied. */
	virtual bool TweakContent(const CRhRdkContent& content, const wchar_t* wszParamName, const class CRhRdkVariant& vValueOld) override;

	/** Call this before 'deleting' one or more top-level contents. This requires the caller to
		merely detach the contents from the document and pass ownership of them to this object.
		It eliminates the need to copy the contents and the need to actually delete them, thus
		saving time and memory. Although the contents are not actually deleted, the user
		still perceives them as deleted because they disappear from the UI when detached.
		\param aContent is an array of pointers to the contents being 'deleted'.
		 If any one of these contents is a child, the function will fail. */
	virtual bool DeleteTopLevelContent(const CRhRdkContentArrayNC& aContent) override;

	/** Call this before deleting one or more contents. This copies the content hierarchies
		into the undo object and is therefore somewhat inefficient. If all the contents are
		top-level, it is better to call DeleteTopLevelContent() instead.
		\param aContent is an array of pointers to the contents being deleted. */
	virtual bool DeleteContent(const CRhRdkContentArray& aContent) override;

	/** Call this before promoting a single content from reference to non-reference.
		\param content is the content being promoted. */
	virtual_su bool PromoteContent(const CRhRdkContent& content) /*override*/; // [SDK_UNFREEZE]

	virtual void DeleteThis(void) override { delete this; }

	/** Emergency virtual function for future expansion. */
	virtual void* EVF(const wchar_t*, void*) override;

private:
	/** \internal For RDK internal use only. */
	class CImpl;
	CImpl* m_pImpl;
};

/** \class CRhRdkContentUndoBlocker

	This is a stack object that prevents content undo records from being created. Sometimes it is
	necessary to change the state of a content without creating an undo record. This can be achieved
	by putting one of these on the stack just before making the change.

	\see CRhRdkContentUndo
*/

class RHRDK_SDK CRhRdkContentUndoBlocker final
{
public:
	CRhRdkContentUndoBlocker();
	~CRhRdkContentUndoBlocker();

private:
	/** \internal For RDK internal use only. */
	class CImpl;
	CImpl* m_pImpl;
};
