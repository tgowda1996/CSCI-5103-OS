#include <iostream>
#include "coremap.h"

unsigned long long CoreMapEntry::num = 0;

CoreMapEntry::CoreMapEntry(int frame_number){
    this->frame_number = frame_number; // duplicated info. Can also be accessed from core map index
    this->page_number = -1; // denotes that the frame is empty
    setTs();
    dirty = false;
    used = true;
}

void CoreMapEntry::setTs(){
    ts = num++;
}

int CoreMapEntry::getFrameNumber() const {
    return frame_number;
}

void CoreMapEntry::setFrameNumber(int frameNumber) {
    frame_number = frameNumber;
}

int CoreMapEntry::getPageNumber() const {
    return page_number;
}

void CoreMapEntry::setPageNumber(int pageNumber) {
    page_number = pageNumber;
}

bool CoreMapEntry::isUsed() const {
    return used;
}

void CoreMapEntry::setIsUsed(bool isUsed) {
    CoreMapEntry::used = isUsed;
}

bool CoreMapEntry::isDirty() const {
    return dirty;
}

void CoreMapEntry::setIsDirty(bool isDirty) {
    CoreMapEntry::dirty = isDirty;
}

int CoreMapEntry::getPerms() {
    return (dirty ? (PROT_READ | PROT_WRITE) : PROT_READ);
}

bool CoreMapEntry::isTooOld(int totalFrames) const {
    if ((num - ts) >= (long long unsigned int) 4*totalFrames) return true;

    return false;
}

void CoreMapEntry::assignNewPage(int page) {
    setPageNumber(page);
    setTs();
    setIsDirty(false);
    setIsUsed(true);
}

unsigned long long CoreMapEntry::getTs(){
    return ts;
}
