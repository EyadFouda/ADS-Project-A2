# ADS-Project-A2

A2. Autocomplete + Ranked Search Engine (Medium)

Theme fit: Natural variant | Topics: tries, heaps, strings, hashing

Assessment: Well balanced and easy to demo. The challenge “collecting completions without
a full subtree traversal” needs a concrete technique so students are not left guessing.

Suggested additions:
• Name the technique: cache the top-K completions at each trie node and update them along
the insertion path, then compare with on-demand subtree traversal plus a bounded heap.
• Add a compressed (radix/Patricia) trie and report memory saved versus the plain trie.
• For fuzzy matching, suggest a BK-tree or bounded edit-distance search over the trie (pruning
branches whose distance exceeds the cutoff) instead of comparing against every word.

Embedded-track variant: Tab-completion for an embedded device shell. Boards running
Zephyr, U-Boot or a custom CLI expose a serial shell with command completion. Students
build the command trie under a strict memory budget (radix trie, fixed node pool), and imple-
ment command history (up/down arrows) with a fixed-size ring buffer: a natural queue/stack
exercise.
