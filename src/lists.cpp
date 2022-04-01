/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_INCLUDE_CONFIG_H)
#include <micro-os-plus/config.h>
#endif // MICRO_OS_PLUS_INCLUDE_CONFIG_H

#include <micro-os-plus/utils/lists.h>
#include <micro-os-plus/diag/trace.h>

// ----------------------------------------------------------------------------

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Waggregate-return"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wc++98-compat"
#endif

namespace micro_os_plus
{
  namespace utils
  {
    // ======================================================================

    /**
     * @class static_double_list_links
     * @details
     * This is the simplest list node, used as base class for other
     * list nodes and as storage for static_double_list,
     * that must be available for any statically constructed
     * objects while still avoiding the 'static initialisation order fiasco'.
     *
     * The idea is to design the object in such a way as to benefit
     * from the standard BSS initialisation, in other words take `nullptr`
     * as starting values.
     */

    /**
     * @details
     * To be fully linked, both pointers must be non null.
     */
    bool
    static_double_list_links::linked (void)
    {
      if (next_ != nullptr && previous_ != nullptr)
        {
          return true;
        }
      assert (next_ == nullptr);
      assert (previous_ == nullptr);
      return false;
    }

    /**
     * @details
     * Update the neighbours to
     * point to each other, skipping the node.
     *
     * For more robustness, to prevent unexpected accesses,
     * the links in the removed node are nullified.
     */
    void
    static_double_list_links::unlink (void)
    {
      // Check if not already unlinked.
      if (!linked ())
        {
          assert (next_ == nullptr);
          assert (previous_ == nullptr);
#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
          trace::printf ("%s() %p nop\n", __func__, this);
#endif
          return;
        }

#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
      trace::printf ("%s() %p \n", __func__, this);
#endif

      // Make neighbours point to each other.
      previous_->next_ = next_;
      next_->previous_ = previous_;

      // Nullify both pointers in the unlinked node.
      clear ();
    }

    // ========================================================================

    /**
     * @class static_double_list
     * @details
     * This is the simplest list, used as base class for scheduler
     * lists that must be available for any statically constructed
     * thread while still avoiding the 'static initialisation order fiasco'.
     *
     * The idea is to design the object in such a way as to benefit
     * from the standard BSS initialisation, in other words take `nullptr`
     * as starting values.
     *
     * This has the downside of requiring additional tests before
     * adding new nodes to the list, to create the initial self
     * links, and when checking if the list is empty.
     */

    /**
     * @details
     * Initialise the mandatory node with links to itself.
     */
    void
    static_double_list::clear (void)
    {
#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
      trace::printf ("%s() @%p\n", __func__, this);
#endif
      head_.next (&head_);
      head_.previous (&head_);
    }

    bool
    static_double_list::empty (void) const
    {
      // If it points to itself, it is empty.
      return (head_.next () == &head_) || uninitialized ();
    }

    static_double_list::iterator
    static_double_list::begin ()
    {
      if (uninitialized ())
        {
          // If this is the first time, initialise the list to empty.
          clear ();
        }
      // return iterator{ static_cast<iterator_pointer> (head_.next ()) };
      return iterator{ head_.next () };
    }

    static_double_list::iterator
    static_double_list::end () const
    {
      // return iterator{ static_cast<iterator_pointer> (
      //     const_cast<static_double_list_links*> (&head_)) };
      return iterator{ const_cast<static_double_list_links*> (&head_) };
    }

    void
    static_double_list::link (reference node)
    {
      if (uninitialized ())
        {
          // If this is the first time, initialise the list to empty.
          clear ();
        }

      // Add node at the end of the list.
      insert_after (node, tail ());
    }

    void
    static_double_list::insert_after (reference node, pointer after)
    {
#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
      trace::printf ("%s() n=%p after %p\n", __func__, &node, after);
#endif

      // Unlinked nodes must have both pointers null.
      // If not, most probably the node was already linked.
      // Or the memory is corrupted.
      assert (node.previous () == nullptr);
      assert (node.next () == nullptr);

      // The `after` node must be linked. Only the `next` pointer is
      // tested, since only it is used.
      assert (after->next () != nullptr);

      // Make the new node point to its neighbours.
      node.previous (after);
      node.next (after->next ());

      // Make the neighbours point to the node. The order is important.
      after->next ()->previous (&node);
      after->next (&node);
    }

    // ========================================================================

    /**
     * @details
     * The initial list status is empty.
     */
    double_list::double_list ()
    {
#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS_CONSTRUCT) \
    || defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
      trace::printf ("%s() @%p \n", __func__, this);
#endif

      clear ();
    }

    /**
     * @details
     * There must be no nodes in the list.
     */
    double_list::~double_list ()
    {
#if defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS_CONSTRUCT) \
    || defined(MICRO_OS_PLUS_TRACE_UTILS_LISTS)
      trace::printf ("%s() @%p \n", __func__, this);
#endif

      assert (empty ());
    }

    void
    double_list::link (reference node)
    {
      // Add node at the end of the list.
      insert_after (node, tail ());
    }

  } // namespace utils
} // namespace micro_os_plus

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
