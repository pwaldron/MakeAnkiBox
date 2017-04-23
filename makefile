all:
	g++ makeAnkiCards.c -std=c++11 -o makeAnkiBox
	g++ alphagram.cpp -o alphagram
	gcc -O2 Blitzkrieg_Trie_Attack_Dawg_Creator_Custom_Character_Set.c -o blitz