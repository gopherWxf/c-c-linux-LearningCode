#include <stdio.h>
#include <stdlib.h>

//b tree


#define M 3//最好是偶数，易于分裂。阶
typedef int KEY_TYPE;

//btree节点
struct btree_node {
    struct btree_node **children;//子树
    KEY_TYPE *keys;//关键字
    int num;//关键字数量
    int leaf;//是否是叶子结点 1yes,0no
};
//btree
struct btree {
    struct btree_node *root;
    int t;
};

//创建一个结点
struct btree_node *btree_create_node(int t, int leaf) {
    struct btree_node *node = (struct btree_node *) calloc(1, sizeof(struct btree_node));
    if (node == NULL)return NULL;

    node->num = 0;
    node->keys = (KEY_TYPE *) calloc(1, (2 * t - 1) * sizeof(KEY_TYPE));
    node->children = (struct btree_node **) calloc(1, (2 * t) * sizeof(struct btree_node *));
    node->leaf = leaf;

    return node;
}

//销毁一个结点
struct btree_node *btree_destroy_node(struct btree_node *node) {
    if (node) {
        if (node->keys) {
            free(node->keys);
        }
        if (node->children) {
            free(node->children);
        }
        free(node);
    }
}

//创建一个btree
void btree_create(btree *T, int t) {
    T->t = t;
    struct btree_node *x = btree_create_node(t, 1);
    T->root = x;
}

//分裂
void btree_split_clild(struct btree *T, struct btree_node *x, int i) {
    //y 需要分裂的结点
    struct btree_node *y = x->children[i];
    //新节点
    struct btree_node *z = btree_create_node(T->t, y->leaf);
    int j = 0;
    z->num = T->t - 1;
    //把y后半部分的key和子树copy到z里
    for (j = 0; j < T->t - 1; j++) {
        z->keys[j] = y->keys[j + T->t];
    }
    if (y->leaf == 0) {
        for (j = 0; j < T->t; j++) {
            z->children[j] = y->children[j + T->t];
        }
    }
    //y结点修改数量
    y->num = T->t - 1;
    //将父节点x增加一个key和子树z
    for (j = x->num; j >= i + 1; j--) {
        x->children[j + 1] = x->children[j];
    }
    x->children[i + 1] = z;
    for (j = x->num - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }
    x->keys[i] = y->keys[T->t - 1];
    x->num++;
}

void btree_insert_nonfull(struct btree *T, struct btree_node *x, KEY_TYPE key) {
    int i = x->num - 1;
    if (x->leaf) {
        while (i >= 0 && x->keys[i] > key) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        x->keys[i + 1] = key;
        x->num++;
    }
    else {
        while (i >= 0 && x->keys[i] > key)i--;
        if (x->children[i + 1]->num == 2 * T->t - 1) {
            btree_split_clild(T, x, i + 1);
            if (key > x->keys[i + 1])i++;
        }
        btree_insert_nonfull(T, x->children[i + 1], key);
    }
}

void btree_insert(struct btree *T, KEY_TYPE key) {
    struct btree_node *root = T->root;
    //如果根结点满了，根节点分裂
    if (root->num == 2 * T->t - 1) {
        btree_node *node = btree_create_node(T->t, 0);
        T->root = node;
        node->children[0] = root;
        btree_split_clild(T, node, 0);
        int i = 0;
        if (key > node->keys[0]) i++;
        btree_insert_nonfull(T, node->children[i], key);

    }
    else {
        btree_insert_nonfull(T, root, key);
    }
}

//{child[idx], key[idx], child[idx+1]}
void btree_merge(btree *T, btree_node *node, int idx) {
    //左右子树
    btree_node *left = node->children[idx];
    btree_node *right = node->children[idx + 1];

    int i = 0;

    //data merge
    left->keys[T->t - 1] = node->keys[idx];
    for (i = 0; i < T->t - 1; i++) {
        left->keys[T->t + i] = right->keys[i];
    }
    if (!left->leaf) {
        for (i = 0; i < T->t; i++) {
            left->children[T->t + i] = right->children[i];
        }
    }
    left->num += T->t;

    //destroy right
    btree_destroy_node(right);

    //node
    for (i = idx + 1; i < node->num; i++) {
        node->keys[i - 1] = node->keys[i];
        node->children[i] = node->children[i + 1];
    }
    node->children[i + 1] = NULL;
    node->num -= 1;

    if (node->num == 0) {
        T->root = left;
        btree_destroy_node(node);
    }
}

void btree_delete_key(btree *T, btree_node *node, KEY_TYPE key) {
    //如果删除的是null直接返回
    if (node == NULL) return;
    int idx = 0, i;
    //找到key的位置
    while (idx < node->num && key > node->keys[idx]) {
        idx++;
    }
    //如果key是内节点
    if (idx < node->num && key == node->keys[idx]) {
        //如果内节点是叶子节点，直接删
        if (node->leaf) {
            for (i = idx; i < node->num - 1; i++) {
                node->keys[i] = node->keys[i + 1];
            }
            node->keys[node->num - 1] = 0;
            node->num--;
            if (node->num == 0) { //如果整个树都被删了
                free(node);
                T->root = NULL;
            }
            return;
        }
            //前面的结点借位
        else if (node->children[idx]->num >= T->t) {
            btree_node *left = node->children[idx];
            node->keys[idx] = left->keys[left->num - 1];
            btree_delete_key(T, left, left->keys[left->num - 1]);
        }
            //后面的结点借位
        else if (node->children[idx + 1]->num >= T->t) {
            btree_node *right = node->children[idx + 1];
            node->keys[idx] = right->keys[0];
            btree_delete_key(T, right, right->keys[0]);
        }
        else {//合并
            btree_merge(T, node, idx);//合并了一个子树上
            btree_delete_key(T, node->children[idx], key);
        }
    }
    else {
        //key不在这层，进入下层
        btree_node *child = node->children[idx];
        if (child == NULL) {
            printf("Cannot del key = %d\n", key);
            return;
        }
        //说明需要借位
        if (child->num == T->t - 1) {
            //left child right三个节点
            btree_node *left = NULL;
            btree_node *right = NULL;
            if (idx - 1 >= 0)
                left = node->children[idx - 1];
            if (idx + 1 <= node->num)
                right = node->children[idx + 1];
            //说明有一个可以借位
            if ((left && left->num >= T->t) || (right && right->num >= T->t)) {
                int richR = 0;
                if (right) {
                    richR = 1;
                }
                //如果右子树比左子树的key多，则richR=1
                if (left && right) {
                    richR = (right->num > left->num) ? 1 : 0;
                }
                //从下一个借
                if (right && right->num >= T->t && richR) {
                    child->keys[child->num] = node->keys[idx];//将父节点的key拿来，拿子树，右节点的第一个子树
                    child->children[child->num + 1] = right->children[0];
                    child->num++;
                    //父节点借右节点的第一个key
                    node->keys[idx] = right->keys[0];
                    //右节点被借位，删除一些东西
                    for (i = 0; i < right->num - 1; i++) {
                        right->keys[i] = right->keys[i + 1];
                        right->children[i] = right->children[i + 1];
                    }
                    right->keys[right->num - 1] = 0;
                    right->children[right->num - 1] = right->children[right->num];
                    right->children[right->num] = NULL;
                    right->num--;
                }
                    //从上一个借
                else {
                    for (i = child->num; i > 0; i--) {
                        child->keys[i] = child->keys[i - 1];
                        child->children[i + 1] = child->children[i];
                    }
                    child->children[1] = child->children[0];
                    //将左子树的最后一个子树拿来
                    child->children[0] = left->children[left->num];
                    //拿父节点的key
                    child->keys[0] = node->keys[idx - 1];
                    child->num++;
                    //父节点那左子树的key
                    node->keys[idx - 1] = left->keys[left->num - 1];
                    left->keys[left->num - 1] = 0;
                    left->children[left->num] = NULL;
                    left->num--;
                }
            }
                //合并
            else if ((!left || (left->num == T->t - 1)) && (!right || (right->num == T->t - 1))) {
                //把node和left合并
                if (left && left->num == T->t - 1) {
                    btree_merge(T, node, idx - 1);
                    child = left;
                }
                    //把node和right合并
                else if (right && right->num == T->t - 1) {
                    btree_merge(T, node, idx);
                }
            }
        }
        //递归下一层
        btree_delete_key(T, child, key);
    }
}


int btree_delete(btree *T, KEY_TYPE key) {
    if (!T->root) return -1;
    btree_delete_key(T, T->root, key);
    return 0;
}

void btree_traverse(btree_node *x) {
    int i = 0;

    for (i = 0; i < x->num; i++) {
        if (x->leaf == 0)
            btree_traverse(x->children[i]);
        printf("%C ", x->keys[i]);
    }

    if (x->leaf == 0) btree_traverse(x->children[i]);
}

void btree_print(btree *T, btree_node *node, int layer) {
    btree_node *p = node;
    int i;
    if (p) {
        printf("\nlayer = %d keynum = %d is_leaf = %d\n", layer, p->num, p->leaf);
        for (i = 0; i < node->num; i++)
            printf("%c ", p->keys[i]);
        printf("\n");
#if 0
        printf("%p\n", p);
        for(i = 0; i <= 2 * T->t; i++)
            printf("%p ", p->childrens[i]);
        printf("\n");
#endif
        layer++;
        for (i = 0; i <= p->num; i++)
            if (p->children[i])
                btree_print(T, p->children[i], layer);
    }
    else printf("the tree is empty\n");
}


int btree_bin_search(btree_node *node, int low, int high, KEY_TYPE key) {
    int mid;
    if (low > high || low < 0 || high < 0) {
        return -1;
    }
    while (low <= high) {
        mid = (low + high) / 2;
        if (key > node->keys[mid]) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }

    return low;
}

int main() {
    btree T = {0};

    btree_create(&T, 3);
    srand(48);

    int i = 0;
    char key[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (i = 0; i < 26; i++) {
        //key[i] = rand() % 1000;
        printf("%c ", key[i]);
        btree_insert(&T, key[i]);
    }
    btree_print(&T, T.root, 0);
    for (i = 0; i < 26; i++) {
        printf("\n---------------------------------\n");
        btree_delete(&T, key[25 - i]);
        //btree_traverse(T.root);
        btree_print(&T, T.root, 0);
    }
}

