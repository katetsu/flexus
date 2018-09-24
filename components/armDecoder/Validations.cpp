// DO-NOT-REMOVE begin-copyright-block 
//
// Redistributions of any form whatsoever must retain and/or include the
// following acknowledgment, notices and disclaimer:
//
// This product includes software developed by Carnegie Mellon University.
//
// Copyright 2012 by Mohammad Alisafaee, Eric Chung, Michael Ferdman, Brian 
// Gold, Jangwoo Kim, Pejman Lotfi-Kamran, Onur Kocberber, Djordje Jevdjic, 
// Jared Smolens, Stephen Somogyi, Evangelos Vlachos, Stavros Volos, Jason 
// Zebchuk, Babak Falsafi, Nikos Hardavellas and Tom Wenisch for the SimFlex 
// Project, Computer Architecture Lab at Carnegie Mellon, Carnegie Mellon University.
//
// For more information, see the SimFlex project website at:
//   http://www.ece.cmu.edu/~simflex
//
// You may not use the name "Carnegie Mellon University" or derivations
// thereof to endorse or promote products derived from this software.
//
// If you modify the software you must place a notice on or within any
// modified version provided or made available to any third party stating
// that you have modified the software.  The notice shall include at least
// your name, address, phone number, email address and the date and purpose
// of the modification.
//
// THE SOFTWARE IS PROVIDED "AS-IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER
// EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO ANY WARRANTY
// THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS OR BE ERROR-FREE AND ANY
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// TITLE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
// BE LIABLE FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO DIRECT, INDIRECT,
// SPECIAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF, RESULTING FROM, OR IN
// ANY WAY CONNECTED WITH THIS SOFTWARE (WHETHER OR NOT BASED UPON WARRANTY,
// CONTRACT, TORT OR OTHERWISE).
//
// DO-NOT-REMOVE end-copyright-block


#include <iostream>
#include <iomanip>
#include <bitset>

#include <core/boost_extensions/intrusive_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <functional>

#include <core/target.hpp>
#include <core/debug/debug.hpp>
#include <core/types.hpp>

#include <components/uArchARM/uArchInterfaces.hpp>
#include <core/qemu/mai_api.hpp>

#include "SemanticInstruction.hpp"
#include "Effects.hpp"
#include "SemanticActions.hpp"
#include "Validations.hpp"
#include <components/uArchARM/systemRegister.hpp>

#define DBG_DeclareCategories armDecoder
#define DBG_SetDefaultOps AddCat(armDecoder)
#include DBG_Control()

namespace narmDecoder {

using namespace nuArchARM;


bool validateXRegister::operator () () {
  if (theInstruction->isSquashed() || theInstruction->isAnnulled()) {
    return true; //Don't check
  }
  if (theInstruction->raised()) {
    DBG_( Tmp, ( << " Not performing register validation for " << theReg << " because of exception. " << *theInstruction ) );
    return true;
  }

  uint64_t flexus = theInstruction->operand< uint64_t > (theOperandCode);
  uint64_t qemu = Flexus::Qemu::Processor::getProcessor(theInstruction->cpu())->readXRegister( theReg );

  DBG_( Tmp, ( << "Validating mapped_reg " << theReg << " flexus=" << std::hex << flexus << " qemu=" << qemu << std::dec << "\n" << std::internal << *theInstruction ) );

  return (flexus == qemu);
}


bool validatePC::operator () () {
  if (theInstruction->isSquashed() || theInstruction->isAnnulled()) {
    return true; //Don't check
  }
  if (theInstruction->raised()) {
    DBG_( Tmp, ( << " Not performing  validation because of exception. " << *theInstruction ) );
    return true;
  }

  uint64_t flexus = theAddr;
  uint64_t qemu = Flexus::Qemu::Processor::getProcessor(theInstruction->cpu())->readPC();

  DBG_( Tmp, ( << "Validating flexus PC=" << std::hex << flexus << " qemu PC=" << qemu << std::dec << "\n" << std::internal << *theInstruction ) );

  return (flexus == qemu);
}

bool validateVRegister::operator () () {

    return true;
//  if (theInstruction->isSquashed() || theInstruction->isAnnulled()) {
//    return true; //Don't check
//  }

//  uint64_t flexus = theInstruction->operand< uint64_t > (theOperandCode);
//  uint64_t simics = Flexus::Qemu::Processor::getProcessor(theInstruction->cpu())->readX( theReg & (~1) );
//  if (theReg & 1) {
//    simics &= 0xFFFFFFFFULL;
//  } else {
//    simics >>= 32;
//  }
//  DBG_( Dev, Condition( flexus != simics) ( << "Validation Mismatch for mapped_reg " << theReg << " flexus=" << std::hex << flexus << " simics=" << simics << std::dec << "\n" << std::internal << *theInstruction ) );
//  return (flexus == simics);
}

bool validateMemory::operator () () {
//  if (theInstruction->isSquashed() || theInstruction->isAnnulled()) {
//    return true; //Don't check
//  }

//  VirtualMemoryAddress flexus_addr(theInstruction->operand< uint64_t > (theAddressCode));
//  if (theInstruction->hasOperand( kUopAddressOffset ) ) {
//    uint64_t offset = theInstruction->operand< uint64_t > (kUopAddressOffset);
//    flexus_addr += offset;
//  }

//  flexus_addr += theAddrOffset;
//  uint64_t flexus_value = theInstruction->operand< uint64_t > (theValueCode);
//  switch (theSize) {
//    case kByte:
//      flexus_value &= 0xFFULL;
//      break;
//    case kHalfWord:
//      flexus_value &= 0xFFFFULL;
//      break;
//    case kWord:
//      flexus_value &= 0xFFFFFFFFULL;
//      break;
//    case kDoubleWord:
//    default:
//      break;
//  }

//  Flexus::Qemu::Processor c = Flexus::Qemu::Processor::getProcessor(theInstruction->cpu());
//  Flexus::Qemu::Translation xlat;
//  xlat.theVaddr = flexus_addr;
//  xlat.theType = Flexus::Qemu::Translation::eLoad;
//  xlat.thePSTATE = c->readRegister( kPSTATE );
//  c->translate(xlat, false);
//  if (xlat.isMMU()) {
//    //bool mmu_ok = Flexus::Qemu::Processor::getProcessor(theInstruction->cpu())->validateMMU();
//    //if (! mmu_ok) {
//    //DBG_( Dev, Condition( ! mmu_ok) ( << "Validation Mismatch for MMUs\n" << std::internal << *theInstruction ) );
//    //Flexus::Qemu::Processor::getProcessor(theInstruction->cpu())->dumpMMU();
//    //}
//    return true;
//  } else if (xlat.thePaddr > 0x400000000LL) {
//    DBG_( Tmp, ( << "Non-memory store " << std::hex << asi << " flexus=" << flexus_value << " Insn: " << *theInstruction ) );
//    return true;
//  } else if (xlat.isTranslating() && !xlat.isSideEffect()) {
//    //uint64_t simics_value = c->readVAddrXendian_QemuImpl( xlat.theVaddr, xlat.theASI, static_cast<int>(theSize) );
//    //DBG_( Dev, Condition( flexus_value != simics_value) ( << "Validation Mismatch for address " << flexus_addr << " flexus=" << std::hex << flexus_value << " simics=" << simics_value << std::dec << "\n" << std::internal << *theInstruction ) );
//    return 1; //(1 || flexus_value == simics_value);
//  } else {
//    DBG_( Tmp, ( << "No validation available for ASI 0x" << std::hex << asi << " flexus=" << flexus_value << " Insn: " << *theInstruction ) );
//    return true;
//  }
    return true;
}

bool validateLegalReturn::operator ()() {
//    if (theInstruction->core()->getCurrentEL() == EL0) {
//        return false;
//    }
    return true;
}


int Align(int x, int y){
    return y * (x / y);
}

bool AArch64ExclusiveMonitorsPass::operator ()() {
//    It is IMPLEMENTATION DEFINED whether the detection of memory aborts happens
//    before or after the check on the local Exclusive Monitor. As a result a failure
//    of the local monitor can occur on some implementations even if the memory
//    access would give an memory abort.

//    eAccType acctype = kAccType_ATOMIC;
//    bool iswrite = true;
//    uint64_t address = theInstruction->operand<uint64_t> (kAddress);

//    bool aligned = (address == Align(address, theSize));
//    if (!aligned){
//        bool secondstage = false;
//        AArch64.Abort(address, AArch64.AlignmentFault(acctype, iswrite, secondstage));
//    }
//    bool passed = AArch64.IsExclusiveVA(address, theCPU, theSize);
//    if (!passed){
//        return false;
//    }
//    memaddrdesc = AArch64.TranslateAddress(address, acctype, iswrite, aligned, theSize);
//    // Check for aborts or debug exceptions
//    if (IsFault(memaddrdesc)){
//        AArch64.Abort(address, memaddrdesc.fault);
//    }
//    passed = IsExclusiveLocal(memaddrdesc.paddress, theCPU, theSize);
//    if (passed){
//        ClearExclusiveLocal(ProcessorID());
//        if (memaddrdesc.memattrs.shareable){
//            passed = IsExclusiveGlobal(memaddrdesc.paddress, theCPU, theSize);
//        }
//    return passed;
    return true;

}
bool SysReg_access::operator ()(){
    SysRegInfo* ri = theInstruction->core()->getSysRegInfo(theOpc0, theOpc1, theOpc2, theCRn, theCRm);
    return (ri->access >> ((theInstruction->core()->currentEL() * 2) + theIsRead)) & 1;
}

bool checkSystemAccess::operator ()(){
    bool unallocated = false;
    bool need_secure = false;
    uint32_t min_EL;

        // Check for unallocated encodings
    switch(theOpc1) {
      case 0: case 1: case 2:
        min_EL = EL1;
        break;
      case 3:
        min_EL = EL0;
        break;
      case 4:
        min_EL = EL2;
        DBG_Assert(false, (<< "we dont model EL2"));

        break;
      case 5:
    //      if !HaveVirtHostExt() then UnallocatedEncoding();
        min_EL = EL2;
        DBG_Assert(false, (<< "we dont model EL2"));
      case 6:
        min_EL = EL3;
        DBG_Assert(false, (<< "we dont model EL3"));

      case  7:
        min_EL = EL1;
        need_secure = true;
    }
    return true;

//        if (theInstruction->core()->currentEL()< min_EL)
//            return false;

    // FIXME -- Figure out how to get the appropirate system reg
    //    (take_trap, target_el) = AArch64.CheckSystemRegisterTraps(op0, op1, crn, crm, op2, read);
    //    if take_trap then
    //        AArch64.SystemRegisterTrap(target_el, op0, op2, op1, crn, rt, crm, read);
}

} //narmDecoder
