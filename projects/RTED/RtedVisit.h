#ifndef RTEDVISITVAR_H
#define RTEDVISITVAR_H

#include "RtedTransformation.h"

// Build an inherited attribute for the tree traversal to test the rewrite mechanism

namespace rted
{
  struct InheritedAttribute
  {
    bool          function;
    bool          isAssignInitializer;
    bool          isArrowExp;
    bool          isAddressOfOp;
    bool          isForStatement;
    bool          isBinaryOp;
    SgStatement*  lastGForLoop;
    SgBinaryOp*   lastBinary;

    InheritedAttribute()
    : function(false),  isAssignInitializer(false), isArrowExp(false),
      isAddressOfOp(false), isForStatement(false), isBinaryOp(false),
      lastGForLoop(NULL), lastBinary(NULL)
    {}
  };


  struct VariableTraversal : public SgTopDownProcessing<InheritedAttribute>
  {
    typedef SgTopDownProcessing<InheritedAttribute> Base;

     explicit
     VariableTraversal(RtedTransformation* t) ;

       // Functions required
    InheritedAttribute evaluateInheritedAttribute (SgNode* astNode, InheritedAttribute inheritedAttribute );

    friend class InheritedAttributeHandler;

  private:
    RtedTransformation* const   transf;

    // should fail when needed
    VariableTraversal();
    VariableTraversal(const VariableTraversal&);
    VariableTraversal& operator=(const VariableTraversal&);
  };
}

#endif
