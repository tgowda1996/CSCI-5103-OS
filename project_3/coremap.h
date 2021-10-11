#ifndef CSCI_5103_PROJECT_3_COREMAP_H
#define CSCI_5103_PROJECT_3_COREMAP_H

#include <sys/mman.h>

class CoreMapEntry {
public:
    CoreMapEntry(){}
    CoreMapEntry(int frame_number);

    void setTs();

    unsigned long long getTs();

    int getFrameNumber() const;

    void setFrameNumber(int frameNumber);

    int getPageNumber() const;

    void setPageNumber(int pageNumber);

    bool isUsed() const;

    void setIsUsed(bool isUsed);

    bool isDirty() const;

    void setIsDirty(bool isDirty);

    /**
     * Returns the permissions of a particular page using the isDirtyFlag
     * @return the right permissions for a particular page
     */
    int getPerms();

    /**
     * Resets the state of the core map entry when a new page is being assigned to the core map
     * @param page the new page that is assigned to the frame
     */
    void assignNewPage(int page);

    /**
     * Checks if a frame is too old and has been resident in the memory for too long
     * @param totalFrames total number of frames in memory
     * @return if the page is too old or not following a particular heuristic.
     */
    bool isTooOld(int totalFrames) const;

private:
    static unsigned long long num;
    int frame_number;
    int page_number;
    unsigned long long ts;
    bool used;
    bool dirty;
};

#endif //CSCI_5103_PROJECT_3_COREMAP_H
