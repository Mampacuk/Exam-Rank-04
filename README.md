# Exam Rank 04
School 42's Exam Rank 04 solution.

## Features
*	Minimal parsing. No memory allocation for the arguments of the commands required. No `ft_strdup()`s. Instead, `argv` is reused. (extremely eco-friendly)
*	No complicated data structures, such as Doubly Linked Lists.
*	No silly enums symbolizing which command preceeded what. No memorizing of this stuff.

## Notes
*	This is no true representative of a legit shell. It executes the commands one after another, but it suffices for the exam.
*	`kill(0, SIGINT)` may be unnecessary; you can _probably_ simplify `ft_exit()` further.
