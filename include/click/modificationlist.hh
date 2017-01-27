#ifndef MIDDLEBOX_MODIFICATIONLIST_HH
#define MIDDLEBOX_MODIFICATIONLIST_HH

/*
 * modificationlist.hh - Class used to store the modifications performed in a
 * packet's structure
 *
 * Romain Gaillard.
 */

#include <click/config.h>
#include <click/glue.hh>
#include <click/memorypool.hh>
#include <click/bytestreammaintainer.hh>

class ByteStreamMaintainer;

/**
 * @brief Structure representing a node in the linked list of modifications
 */
struct ModificationNode
{
        unsigned int position; /** Position of the modification */
        int offset; /** Offset of the modification (positive if bytes are added
                        or negative if bytes are removed) */
        struct ModificationNode* next; /** Pointer to the next node */
};

/** @class ModificationList
 * @brief Class used to store the modifications performed in a packet's structure
 *
 * This class is used to store the modifications made in the structure of a
 * packet. It stores the positions at which bytes are inserted and removed
 * so that they can be committed when the packet is in its final state.
 * The modifications are committed to a ByteStreamMaintainer.
 */
class ModificationList
{
    // TCPOut is the only component allowed to commit modifications
    // It is thus a friend which can access the private method "commit"
    friend class TCPOut;

public:
    /** @brief Construct a ModificationList
     * @param poolNodes A pointer to a MemoryPool for the nodes of type
     * struct ModificationNode.
     */
    ModificationList(MemoryPool<struct ModificationNode>* poolNodes);
    ~ModificationList();
    /** @brief Print the current state of the list in the console using
     * click_chatter
     */
    void printList();
    /** @brief Add a modification in the list
     * @param position The position at which the modification occurs
     * @param offset The offset representing the modification (a positive
     * number if bytes are added and a negative number if bytes are removed)
     * @return True if the modification has been taken into account and false
     * if the list does not accept any new modification because a commit
     * has been made.
     */
    bool addModification(unsigned int position, int offset);

    bool isCommitted()
    {
        return committed;
    }

private:
    /** @brief Indicate if two integers have the same sign
     * @param first The first integer
     * @param second The second integer
     * @return  True if both integers have the same sign
     */
    bool sameSign(int first, int second);
    /** @brief Merge nodes that represent overlapping modifications of the
     * same kind
     */
    void mergeNodes();
    /** @brief Commit the modifications in the ByteStreamMaintainer.
     * @param maintainer The ByteStreamMaintainer in which the modifications
     * will be committed
     */
    void commit(ByteStreamMaintainer& maintainer);
    /** @brief Clear the modification list
     */
    void clear();

    MemoryPool<struct ModificationNode> *poolNodes;
    struct ModificationNode *head;
    bool committed; /** Becomes true when a commit is made */
};

#endif