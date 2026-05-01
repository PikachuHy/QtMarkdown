//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_PAINTPASS_H
#define QTMARKDOWN_PAINTPASS_H
#include "mddef.h"
#include "PaintRecord.h"
#include "Instruction.h"

namespace md::render {

class PaintPass {
public:
    explicit PaintPass(const PaintRecordList& records);
    InstructionPtrList execute();
private:
    const PaintRecordList& m_records;
};

}

#endif // QTMARKDOWN_PAINTPASS_H
