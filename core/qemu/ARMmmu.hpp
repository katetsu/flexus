#ifndef FLEXUS_SIMICS_ARM_MMU_HPP_INCLUDED
#define FLEXUS_SIMICS_ARM_MMU_HPP_INCLUDED

#include "ARMTranslationGranules.hpp"
#include "TTResolvers.hpp"
#include "bitUtilities.hpp"

#include <core/types.hpp>
#include <components/CommonQEMU/Slices/Translation.hpp>
#include <components/CommonQEMU/Slices/TranslationState.hpp>
#include "mmuRegisters.h"

using Flexus::SharedTypes::Translation;
using Flexus::SharedTypes::VirtualMemoryAddress;

//namespace Flexus {
//namespace Qemu {

namespace MMU {

#define VADDR_WIDTH 48

typedef unsigned long long address_t;
typedef unsigned char asi_t;
typedef unsigned long long tte_raw_t;

class TTEDescriptor
{
    public:
        // helper functions for reading ARM64 descriptors
        bool isValid();
        bool isTableEntry();
        bool isBlockEntry();

    private:
        tte_raw_t rawDescriptor;
};

/* RAW DESCRIPTOR FORMATS ON AARCH64 REFERENCE MANUAL - SECTION D4.3 - D4-2061 */
// INVALID ENTRY bit[0] = 0
/* VALID BLOCK ENTRY: For level 1, n = 42, level 2, n = 29.
 * [ 63:51 | ---- | 50:48 | 47:n     | n-1:16 | 15:12  -- | 11:2 ----- | 1 | 0 ] 
 * [ upper attrs. | res0  | OA[47:n] |  res0  | OA[51:48] | low. attrs | 0 | 1 ]
 *
 * VALID TABLE ENTRY:
 * [ 63 ---- | 62:61 - | 60 ---- | 59 ----- | 58:51  | 50:48 |-----  47:16  ----- |----- 15:12 ------ | 11:2 - | 1 | 0 ]
 * [ NSTable | APTable | XNTable | PXNTable | IGNORE | res0  | Next-level [47:16] | Next-level[51:48] | IGNORE | 1 | 1 ]
 * - entries for NS,AP,XN,PXN only valid in Stage1, res0 in Stage 2
 */

// Msutherl - antiquated + orphaned, needs to go soon
typedef unsigned long long tte_tag;
typedef unsigned long long tte_data;

class mmu_t {
    public:
        /* Msutherl - jun'18
         * - Adds system registers for controlling table walk
         * - TLB tags/data are refactored into Flexus state 
         */
        mmu_regs_t mmu_regs;
        bit_configs_t aarch64_bit_configs;
        std::shared_ptr<TranslationGranule> Gran0;
        std::shared_ptr<TranslationGranule> Gran1;

        /* Accessors that get higher data about this MMU *
         * e.g., is it enabled, what sizes does it implement....
         * - ALL the references for the magic numbers come from the AARCH64 Manual,
         *      and have page #s documented re in ARM-mmu.txt (parsa wiki).
         * - Some of them are in mmuRegisters.h as well.
         */
        bool is4KGranuleSupported();
        bool is16KGranuleSupported();
        bool is64KGranuleSupported();
        unsigned getGranuleSize(unsigned granuleNum);
        unsigned parsePASizeFromRegs();
        unsigned getIAOffsetValue(bool isBRO);

        int checkBR0RangeForVAddr(VirtualMemoryAddress& theVA) const ;
        uint8_t getInitialLookupLevel( bool& isBR0 ) const ;
        uint8_t getIASize(bool isBR0) const ;
        uint8_t getPAWidth(bool isBR0) const ;

    public: /* Functions that are called externally from TLBs, uArch, etc.... */
        void initRegsFromQEMUObject(mmu_regs_t* qemuRegs);
        bool IsExcLevelEnabled(uint8_t elToValidate) const ;
        void setupAddressSpaceSizesAndGranules(void);

    private:
        void setupBitConfigs();
};

typedef enum {
  CLASS_ASI_PRIMARY        = 0,
  CLASS_ASI_SECONDARY      = 1,
  CLASS_ASI_NUCLEUS        = 2,
  CLASS_ASI_NONTRANSLATING = 4
} asi_class_t;

typedef enum {
  MMU_TRANSLATE = 0,  /* for 'normal' translations */
  MMU_TRANSLATE_PF,   /* translate but don't trap */
  MMU_DEMAP_PAGE
} mmu_translation_type_t;

typedef enum {
  MMU_TSB_8K_PTR = 0,
  MMU_TSB_64K_PTR
} mmu_ptr_type_t;

/* taken from simics arm_exception_type */
typedef enum {
  no_exception                     = 0x000,
  /* */
  instruction_access_exception     = 0x008,
  instruction_access_mmu_miss      = 0x009,
  instruction_access_error         = 0x00a,
  /* */
  data_access_exception            = 0x030,
  data_access_mmu_miss             = 0x031,
  data_access_error                = 0x032,
  data_access_protection           = 0x033,
  mem_address_not_aligned          = 0x034,
  /* */
  priveledged_action               = 0x037,
  /* */
  fast_instruction_access_MMU_miss = 0x064,
  fast_data_access_MMU_miss        = 0x068,
  fast_data_access_protection      = 0x06c
  /* */
} mmu_exception_t;

typedef enum {
  mmu_access_load  = 1,
  mmu_access_store = 2
} mmu_access_type_t;

typedef struct mmu_access {
  address_t va;     /* virtual address */
  asi_t asi;         /* ASI */
  mmu_access_type_t type;        /* load or store */
  mmu_reg_t val;  /* store value or load result */
} mmu_access_t;

/* prototypes */
/*
bool mmu_is_cacheable(tte_data data);
bool mmu_is_sideeffect(tte_data data);
bool mmu_is_xendian(tte_data data);
address_t mmu_make_paddr(tte_data data, address_t va);

address_t mmu_translate(mmu_t * mmu,
                        unsigned int is_fetch,
                        address_t va,
                        unsigned int asi_class,
                        unsigned int asi,
                        unsigned int nofault,
                        unsigned int priv,
                        unsigned int access_type,
                        mmu_exception_t * except,
                        mmu_translation_type_t trans_type);

tte_data mmu_lookup(mmu_t * mmu,
                    unsigned int is_fetch,
                    address_t va,
                    unsigned int asi_class,
                    unsigned int asi,
                    unsigned int nofault,
                    unsigned int priv,
                    unsigned int access_type,
                    mmu_exception_t * except,
                    mmu_translation_type_t trans_type);

address_t mmu_generate_tsb_ptr(address_t va,
                               mmu_ptr_type_t type,
                               address_t tsb_base_in,
                               unsigned int tsb_split,
                               unsigned int tsb_size,
                               mmu_reg_t tsb_ext);

void mmu_access(mmu_t * mmu, mmu_access_t * access);

int fm_compare_regs(mmu_regs_t* a, mmu_regs_t * b, const char * who);
int fm_compare_mmus(mmu_t * a, mmu_t * b);
*/
void fm_print_mmu_regs(mmu_regs_t* mmu);
} //end namespace MMU
//#endif
//} //end Namespace Qemu
//} //end namespace Flexus

//#if FLEXUS_TARGET_IS(arm)
#include <core/qemu/mai_api.hpp>
namespace MMU {
}

#endif //FLEXUS_SIMICS_MAI_API_HPP_INCLUDED
