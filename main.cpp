#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>

using ull = unsigned long long;

const ull T = 5;


struct TKV {
    char* Key;
    ull Val;
};


class TNode {
public:
    friend class TBTree;

    TNode(ull facT);
    void Insert(TNode** root ,TKV& elemToInsert);
    TKV* Search(char* keyToSearch);
    void Delete(char* keyToDelete);
    void DeleteTree();
    bool NSerialize(FILE* f, TNode* node);
    bool NDeserialize(FILE* f, TNode* node);
    ~TNode();

private:
    ull t;
    bool isLeaf;
    unsigned elemsNum;
    TKV* elems;
    TNode** children;
    int FindKey(char* keyToSearch);
    TKV& FindMax();
    TKV& FindMin();
    void InsertNonFull(TKV& elemToInsert);
    void DeleteFromLeaf(int index);
    void DeleteFromNonLeaf(int index);
    void Fill(int index);
    void BorrowFromPrev(int index);
    void BorrowFromNext(int index);
    void Split(int i);
    void Merge(int index);
};


class TBTree {
private:
    ull t;
    TNode* root;

public:
    TBTree(ull facT);
    void Insert(TKV& elemToInsert);
    void RemoveElement(char* keyToDelete);
    TKV* SearchWord(char* keyToSearch);
    bool Serialize(const char* pathToFile);
    bool Deserialize(const char* pathToFile);
    ~TBTree();
};


TBTree::TBTree(ull facT) {
    t = facT;
    root = new TNode(t);
}


void TBTree::Insert(TKV& elemToInsert) {
    root->Insert(&root, elemToInsert);
}


TKV* TBTree::SearchWord(char* keyToSearch) {
    if (root != nullptr) {
        return root->Search(keyToSearch);
    } else {
        return nullptr;
    }
}


void TBTree::RemoveElement(char* keyToDelete) {
    root->Delete(keyToDelete);

    if (root->elemsNum == 0) {
        TNode* temp = root;
        if (root->isLeaf == false) {
            root = root->children[0];
            temp->children[0] = nullptr;
            delete temp;
        }
    }
}


bool TBTree::Serialize(const char* pathToFile) {
    FILE* f = fopen(pathToFile, "wb");

    if (f == nullptr) {
        return false;
    }

    if (fwrite(&t, sizeof(t), 1, f) != 1) {
        return false;
    }

    bool ans = root->NSerialize(f, root);

    fclose(f);

    return ans;
}


bool TBTree::Deserialize(const char* pathToFile) {
    FILE* f = fopen(pathToFile, "rb");

    if (f == nullptr) {
        return false;
    }

    if (fread(&t, sizeof(t), 1, f) != 1) {
        return false;
    }

    TNode* rootNew = new TNode(t);
    bool ans = root->NDeserialize(f, rootNew);

    fclose(f);

    if (ans) {
        root->DeleteTree();
        root = rootNew;

        return true;
    } else {
        rootNew->DeleteTree();

        return false;
    }
}


TBTree::~TBTree() {
    root->DeleteTree();
}


void Swap(char* &x, char* &y) {
    char* temp = x;
    x = y;
    y = temp;
}


TNode::TNode(ull facT) {
    elemsNum = 0;
    isLeaf = true;
    t = facT;
    elems = (TKV*)malloc(sizeof(TKV) * (2 * facT - 1));
    children = (TNode**)malloc(sizeof(TNode*) * 2 * facT);
    for (int i = 0; i < 2 * facT; ++i) {
        children[i] = nullptr;
        if (i < 2 * facT - 1) {
            elems[i].Key = nullptr;
        }
    }
}


TKV* TNode::Search(char* keyToSearch) {
    int i = 0;

    while (i < elemsNum && strcmp(keyToSearch, elems[i].Key) > 0) {
        ++i;
    }

    if (i < elemsNum && strcmp(keyToSearch, elems[i].Key) == 0){
        return &elems[i];

    } else if (isLeaf == true) {
        return nullptr;

    } else {
        return children[i]->Search(keyToSearch);
    }
}



void TNode::Split(int i) {

    TNode* z = new TNode(t);
    TNode* y = children[i];
    z->isLeaf = y->isLeaf;
    z->elemsNum = t - 1;

    for (int j = 0; j < t - 1; ++j) {
        Swap(z->elems[j].Key, y->elems[j + t].Key);
        z->elems[j].Val = y->elems[j + t].Val;
    }

    if (y->isLeaf == false) {
        for (int j = 0; j < t; ++j) {
            z->children[j] = y->children[j + t];
        }
    }

    y->elemsNum = t - 1;

    for (int j = elemsNum; j >= i + 1; --j) {
        children[j + 1] = children[j];
    }

    children[i + 1] = z;

    for (int j = elemsNum - 1; j >= i; --j) {
        Swap(elems[j + 1].Key, elems[j].Key);
        elems[j + 1].Val = elems[j].Val;
    }

    Swap(elems[i].Key, y->elems[t - 1].Key);
    elems[i].Val = y->elems[t - 1].Val;
    ++elemsNum;
}


void TNode::Merge(int index) {
    int i;
    TNode* child = children[index];
    TNode* brother = children[index + 1];

    char *temp = child->elems[t - 1].Key;
    child->elems[t - 1].Key = elems[index].Key;
    elems[index].Key = temp;
    child->elems[t - 1].Val = elems[index].Val;

    for (i = 0; i < brother->elemsNum; ++i) {
        Swap(child->elems[i + t].Key, brother->elems[i].Key);
        child->elems[i + t].Val = brother->elems[i].Val;
    }

    if (child->isLeaf == false) {
        for(i = 0; i <= brother->elemsNum; ++i) {
            child->children[i + t] = brother->children[i];
        }
    }


    for (i = index + 1; i < elemsNum; ++i) {
        Swap(elems[i - 1].Key, elems[i].Key);
        elems[i - 1].Val = elems[i].Val;
    }

    for (i = index + 2; i <= elemsNum; ++i) {
        children[i - 1] = children[i];
    }

    children[elemsNum] = nullptr;
    child->elemsNum += brother->elemsNum + 1;
    --elemsNum;
    delete brother;
}


void TNode::Insert(TNode** root, TKV& elemToInsert) {
    if (elemsNum == 2 * t - 1) {
        TNode* s = new TNode(t);
        *root = s;
        s->isLeaf = false;
        s->elemsNum = 0;
        s->children[0] = this;
        s->Split(0);
        s->InsertNonFull(elemToInsert);
    } else {
        (*root)->InsertNonFull(elemToInsert);
    }
}


void TNode::InsertNonFull(TKV& elemToInsert) {
    int i = elemsNum - 1;

    if (isLeaf) {
        while(i >= 0 && strcmp(elemToInsert.Key, elems[i].Key) < 0) {
            Swap(elems[i + 1].Key, elems[i].Key);
            elems[i + 1].Val = elems[i].Val;
            --i;
        }

        elems[i + 1].Key = (char *)malloc(sizeof(char) * strlen(elemToInsert.Key) + 1);
        strcpy(elems[i + 1].Key, elemToInsert.Key);
        elems[i + 1].Val = elemToInsert.Val;
        ++elemsNum;
        std::cout << "OK" << std::endl;
    } else {
        while(i >= 0 && 0 > strcmp(elemToInsert.Key, elems[i].Key)) {
            --i;
        }
        ++i;
        if(children[i]->elemsNum == 2 * t - 1) {
            Split(i);
            if (strcmp(elemToInsert.Key, elems[i].Key) > 0) {
                ++i;
            }
        }
        children[i]->InsertNonFull(elemToInsert);
    }
}


int TNode::FindKey(char* keyToSearch) {
    unsigned first = 0;
    unsigned last = elemsNum;
    while (first < last) {
        unsigned mid = first + (last - first) / 2;
        if (strcmp(keyToSearch, elems[mid].Key) <= 0) {
            last = mid;
        } else {
            first = mid + 1;
        }
    }
    return last;
}


void TNode::DeleteFromLeaf(int index) {
    for (int i = index + 1; i < elemsNum; ++i) {
        Swap(elems[i-1].Key, elems[i].Key);
        elems[i-1].Val = elems[i].Val;
    }
    --elemsNum;
    free(elems[elemsNum].Key);
    elems[elemsNum].Key = nullptr;
    std::cout << "OK" << std::endl;
}

TKV& TNode::FindMax() {
    TNode* temp = this;
    while (temp->isLeaf == false) {
        temp = temp->children[temp->elemsNum];
    }
    return temp->elems[temp->elemsNum - 1];
}


TKV& TNode::FindMin() {
    TNode* temp = this;
    while (temp->isLeaf == false) {
        temp = temp->children[0];
    }
    return temp->elems[0];
}


void TNode::Fill(int index) {
    if (index != 0 && children[index - 1]->elemsNum >= t) {
        BorrowFromPrev(index);

    } else if (index != elemsNum && children[index + 1 ]->elemsNum >= t) {
        BorrowFromNext(index);

    } else {
        if (index != elemsNum) {
            Merge(index);
        } else {
            Merge(index - 1);
        }
    }
}


void TNode::BorrowFromPrev(int index) {
    TNode *child = children[index];
    TNode *brother = children[index - 1];

    for (int i = child->elemsNum - 1; i >= 0; --i) {
        Swap(child->elems[i + 1].Key, child->elems[i].Key);
        child->elems[i + 1].Val = child->elems[i].Val;
    }

    if (child->isLeaf == false) {
        for(int i = child->elemsNum; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    Swap(child->elems[0].Key, elems[index - 1].Key);
    child->elems[0].Val = elems[index - 1].Val;

    if (isLeaf == false) {
        child->children[0] = brother->children[brother->elemsNum];
    }

    Swap(elems[index - 1].Key, brother->elems[brother->elemsNum - 1].Key);
    elems[index - 1].Val = brother->elems[brother->elemsNum - 1].Val;

    ++child->elemsNum;
    --brother->elemsNum;

    return;
}


void TNode::BorrowFromNext(int index) {
    TNode* child = children[index];
    TNode* brother = children[index + 1];

    Swap(child->elems[(child->elemsNum)].Key, elems[index].Key);
    child->elems[(child->elemsNum)].Val = elems[index].Val;

    if (child->isLeaf == false) {
        child->children[(child->elemsNum) + 1] = brother->children[0];
    }

    Swap(elems[index].Key, brother->elems[0].Key);
    elems[index].Val = brother->elems[0].Val;

    for (int i = 1; i < brother->elemsNum; ++i) {
        Swap(brother->elems[i-1].Key, brother->elems[i].Key);
        brother->elems[i-1].Val = brother->elems[i].Val;
    }

    if (brother->isLeaf == false) {
        for(int i = 1; i <= brother->elemsNum; ++i)
            brother->children[i - 1] = brother->children[i];
    }

    child->elemsNum += 1;
    brother->elemsNum -= 1;

    return;
}


void TNode::DeleteFromNonLeaf(int index) {
    char array[257];
    strcpy(array, elems[index].Key);

    if (children[index]->elemsNum >= t) {
        TKV &temp = children[index]->FindMax();
        free(elems[index].Key);
        elems[index].Key = (char*)malloc(sizeof(char) * strlen(temp.Key) + 1);
        strcpy(elems[index].Key, temp.Key);
        elems[index].Val = temp.Val;
        children[index]->Delete(temp.Key);

    } else if (children[index + 1]->elemsNum >= t) {
        TKV& temp = children[index + 1]->FindMin();
        free(elems[index].Key);
        elems[index].Key = (char*)malloc(sizeof(char) * strlen(temp.Key) + 1);
        strcpy(elems[index].Key, temp.Key);
        elems[index].Val = temp.Val;
        children[index + 1]->Delete(temp.Key);

    } else {
        Merge(index);
        children[index]->Delete(array);
    }
}


void TNode::Delete(char* keyToDelete) {
    int index = FindKey(keyToDelete);
    if (index < elemsNum && strcmp(keyToDelete, elems[index].Key) == 0) {
        if (isLeaf) {
            DeleteFromLeaf(index);
        } else {
            DeleteFromNonLeaf(index);
        }

    } else {
        if (isLeaf) {
            std::cout << "NoSuchWord" << std::endl;
            return;
        }

        bool flag = (index == elemsNum) ? true : false;

        if (children[index]->elemsNum < t) {
            Fill(index);
        }

        if (flag == true && index > elemsNum) {
            children[index - 1]->Delete(keyToDelete);
        } else {
            children[index]->Delete(keyToDelete);
        }
    }
    return;
}


bool TNode::NSerialize(FILE* f, TNode* node) {
    if (fwrite(&node->elemsNum, sizeof(node->elemsNum), 1, f) != 1) {
        return false;
    }

    if (fwrite(&node->isLeaf, sizeof(node->isLeaf), 1, f) != 1) {
        return false;
    }

    for (unsigned i = 0; i < node->elemsNum; ++i) {
        const TKV* data = &node->elems[i];
        const size_t keyLen = strlen(data->Key);
        const char* keyToSave = data->Key;

        if (fwrite(&keyLen, sizeof(keyLen), 1, f) != 1) {
            return false;
        }
        if (fwrite(keyToSave, sizeof(char), keyLen, f) != keyLen) {
            return false;
        }
        if (fwrite(&data->Val, sizeof(data->Val), 1, f) != 1) {
            return false;
        }
    }

    if (node->isLeaf == false) {
        for (unsigned i = 0; i < node->elemsNum + 1; ++i) {
            if (NSerialize(f, node->children[i]) == false) {
                return false;
            }
        }
    }

    return true;
}


bool TNode::NDeserialize(FILE* f, TNode* node) {
    char copyKey[257];

    if (fread(&node->elemsNum, sizeof(node->elemsNum), 1, f) != 1) {
        return false;
    }
    if (fread(&node->isLeaf, sizeof(node->isLeaf), 1, f) != 1) {
        return false;
    }

    for (unsigned i = 0; i < node->elemsNum; ++i) {
        TKV* data = &node->elems[i];
        size_t keyLen = 0;

        if (fread(&keyLen, sizeof(keyLen), 1, f) != 1) {
            return false;
        }
        if (fread(copyKey, sizeof(char), keyLen, f) != keyLen) {
            return false;
        }
        if (fread(&data->Val, sizeof(data->Val), 1, f) != 1) {
            return false;
        }

        copyKey[keyLen] = '\0';
        data->Key = (char *)malloc(sizeof(char) * keyLen + 1);
        strcpy(data->Key, copyKey);
    }

    if (node->isLeaf == false) {
        for (unsigned i = 0; i < node->elemsNum + 1; ++i) {
            node->children[i] = new TNode(t);

            if (NDeserialize(f, node->children[i]) == false) {
                return false;
            }
        }
    }

    return true;
}


TNode::~TNode() {
    for (unsigned i = 0; i < elemsNum; ++i) {
        free(elems[i].Key);
    }
    free(elems);
    free(children);
}


void TNode::DeleteTree() {
    TNode *tmp = this;
    if (tmp == nullptr) {
        return;
    }
    for (unsigned i = 0; i < tmp->elemsNum + 1; ++i) {
        tmp->children[i]->DeleteTree();
    }
    delete(tmp);
}


char CharToLower(char c) {
    return isupper(c) ? c - 'A' + 'a' : c;
}



void Parsing(char* action, char* inputStr, ull* value) {
    char ch;
    unsigned i = 0;

    ch = getchar();

    if (ch == EOF) {
        *action = 'Q';
        return;
    }

    if (ch == '+' || ch == '-') {
        *action = ch;
        getchar();
        while (true) {
            ch = CharToLower(getchar());
            if (isalpha(ch) == 0) {
                break;
            } else {
                inputStr[i++] = ch;
            }
        }
        inputStr[i] = '\0';

        if (*action == '+') {
            *value = 0;

            while ((ch = getchar()) != '\n') {
                *value = (*value) * 10 + ch - '0';
            }
        }

    } else if (ch == '!') {
        getchar();
        while ((ch = getchar()) != ' ') {
            inputStr[i++] = ch;
        }
        inputStr[i] = '\0';
        i = 0;
        *action = inputStr[0];
        while((ch = getchar()) != '\n') {
            inputStr[i++] = ch;
        }
        inputStr[i] = '\0';

    } else {
        *action = 'B';
        inputStr[0] = CharToLower(ch);
        i++;
        while((ch = getchar()) != '\n') {
            inputStr[i++] = CharToLower(ch);
        }
        inputStr[i] = '\0';
    }
}

int main(void) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    TBTree tree(T);
    TKV elemToInsert;
    char inputStr[257];
    char action;
    ull value;

    while (true) {
        Parsing(&action, inputStr, &value);

        if (action == 'Q') {
            break;
        }

        switch (action) {
            case '+':
                elemToInsert.Key = (char*)malloc(sizeof(char) * strlen(inputStr) + 1);
                strcpy(elemToInsert.Key, inputStr);
                elemToInsert.Val = value;

                if (tree.SearchWord(inputStr) == nullptr) {
                    tree.Insert(elemToInsert);
                } else {
                    std::cout << "Exist" << std::endl;
                }
                free(elemToInsert.Key);
                break;

            case '-':
                tree.RemoveElement(inputStr);
                break;

            case 'S': {
                std::ofstream fileToSave(inputStr);
                if (!fileToSave.is_open()) {
                    std::cout << "ERROR: Cannot save to this file" << std::endl;
                } else {
                    std::cout << ((tree.Serialize(inputStr) ? "OK" : "ERROR: Cannot create a file")) << std::endl;
                }
                break;
            }

            case 'L': {
                std::ifstream fileToLoad(inputStr);
                if (!fileToLoad.is_open()) {
                    std::cout << "ERROR: Cannot read from this file" << std::endl;
                } else {
                    std::cout << ((tree.Deserialize(inputStr) ? "OK" : "ERROR: Cannot load the file")) << std::endl;
                }
                break;
            }

            case 'B':
                if (tree.SearchWord(inputStr) != nullptr) {
                    std::cout << "OK: " << tree.SearchWord(inputStr)->Val << std::endl;
                } else {
                    std::cout << "NoSuchWord" << std::endl;
                }
                break;
        }
    }
}