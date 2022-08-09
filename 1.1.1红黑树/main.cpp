#include <iostream>

#define RED 0
#define BLACK 1

typedef int KEY_TYPE;

typedef struct _rbtree_node {
    unsigned char color;//颜色
    struct _rbtree_node *left;//左子树
    struct _rbtree_node *right;//右子树
    struct _rbtree_node *parent;//父结点
    KEY_TYPE key;
    void *value;

} rbtree_node;//红黑树结点

typedef struct _rbtree {
    rbtree_node *root;//根结点
    rbtree_node *nil;//通用叶子结点
} rbtree;//红黑树

//左旋leftRotate(T,x)---中右->左中
//降低X结点的高度，提高X结点右结点（即Y）的高度。
void rbtree_left_rotate(rbtree *T, rbtree_node *x) {
    rbtree_node *y = x->right;
    //1
    x->right = y->left;//x的右子树指向y的左子树
    if (y->left != T->nil) {
        y->left->parent = x;//y的左子树的父节点指向x
    }
    //2
    y->parent = x->parent;//y的父结点指向x的父结点
    if (x->parent == T->nil) {//如果x是根结点
        T->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;//本来指向x结点的父指针，改成指向y
    } else {
        x->parent->right = y;
    }
    //3
    y->left = x;//y的左子树指向x结点
    x->parent = y;//x的父节点指向y
}

//右旋
//copy左旋的代码
//左改成右，右改成左
//x改成y，y改成x
void rbtree_right_rotate(rbtree *T, rbtree_node *y) {
    rbtree_node *x = y->left;
    //1
    y->left = x->right;
    if (x->right != T->nil) {
        x->right->parent = y;
    }
    //2
    x->parent = y->parent;
    if (y->parent == T->nil) {
        T->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    //3
    x->right = y;
    y->parent = x;
}

//修复第4条性质
void rbtree_insert_fixup(rbtree *T, rbtree_node *z) {
    while (z->parent->color == RED) {//父结点是红色的，需要调整
        if (z->parent == z->parent->parent->left) {//如果父结点是爷结点是左子树
            rbtree_node *y = z->parent->parent->right;//叔结点
            if (y->color == RED) {//叔结点是红色的
                //先变色，叔,父变黑
                z->parent->color = BLACK;
                y->color = BLACK;
                //爷结点变红
                z->parent->parent->color = RED;
                //下面的调整完了，调整上面
                z = z->parent->parent;
            } else {//叔父结点是黑色
                if (z == z->parent->right) {//新节点是在右边
                    z = z->parent;
                    rbtree_left_rotate(T, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtree_right_rotate(T, z->parent->parent);
            }
        } else {
            // 如果父结点是爷结点是右子树
            rbtree_node *y = z->parent->parent->left;//叔父结点
            if (y->color == RED) {//叔父结点是红色的
                //先变色，叔,父变黑
                z->parent->color = BLACK;
                y->color = BLACK;
                //爷结点变红
                z->parent->parent->color = RED;
                //下面的调整完了，调整上面
                z = z->parent->parent;
            } else {//叔父结点是黑色
                if (z == z->parent->left) {//新节点是在左边
                    z = z->parent;
                    rbtree_right_rotate(T, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtree_left_rotate(T, z->parent->parent);
            }
        }
    }
    //最后别忘了根节点始终是黑色
    T->root->color = BLACK;
}

//因为传入的key可能是字符，可能是整形，所以要提取出来
//这里可以看出，其实可以封装成一个模板
int key_compare(KEY_TYPE a, KEY_TYPE b) {
    //这里假设是int
    if (a > b) {
        return 1;
    } else if (a < b) {
        return -1;
    } else {
        return 0;
    }
}

void rbtree_insert(rbtree *T, rbtree_node *z) {
    //找位置
    rbtree_node *x = T->root;
    rbtree_node *y = T->nil;//y是x的父节点
    while (x != T->nil) {//二分找位置
        y = x;
        if (key_compare(z->key, x->key) < 0) {
            x = x->left;
        } else if (key_compare(z->key, x->key) > 0) {
            x = x->right;
        } else {
            //如果key相等，看自己的业务情景
            //重复插入可以不修改直接退出，可以修改val
            return;
        }
    }
    //插入
    z->parent = y;
    if (y == T->nil) {
        T->root = z;
    } else if (key_compare(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    z->left = T->nil;
    z->right = T->nil;
    z->color = RED;
    //维护红黑树
    rbtree_insert_fixup(T, z);
}


rbtree_node *rbtree_maxi(rbtree *T, rbtree_node *x) {
    while (x->right != T->nil) {
        x = x->right;
    }
    return x;
}


void rbtree_delete_fixup(rbtree *T, rbtree_node *x) {
    //如果x是红色，变成黑色，如果x是黑色，需要调整
    while ((x != T->root) && (x->color == BLACK)) {
        //当前结点是父结点的左子树
        if (x == x->parent->left) {
            //兄弟节点
            rbtree_node *w = x->parent->right;
            // 情况1：兄弟节点为红色
            if (w->color == RED) {
                // 兄弟节点变成黑色
                w->color = BLACK;
                // 父节点变成红色
                x->parent->color = RED;
                // 父节点做左旋
                rbtree_left_rotate(T, x->parent);
                //将兄弟结点调整为父节点的右子树
                w = x->parent->right;
            }
            // 情况2：兄弟节点是黑色的，且兄弟的左孩子和右孩子都是黑色
            if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
                // 兄弟节点变成红色
                w->color = RED;
                // 轴心结点变为父节点
                x = x->parent;
            } else {
                // 情况3：x的兄弟节点是黑色的，兄弟的左孩子是红色，右孩子是黑色
                if (w->right->color == BLACK) {
                    // 将左孩子涂黑
                    w->left->color = BLACK;
                    // 将兄弟节点变红
                    w->color = RED;
                    // 对兄弟节点右旋
                    rbtree_right_rotate(T, w);
                    //将兄弟结点调整为父节点的右子树
                    w = x->parent->right;
                }
                // 情况4：x的兄弟节点是黑色；x的兄弟节点的右孩子是红色的
                // 将兄弟节点换成父节点的颜色
                w->color = x->parent->color;
                // 把父节点和兄弟节点的右孩子涂黑
                x->parent->color = BLACK;
                w->right->color = BLACK;
                // 对父节点做左旋
                rbtree_left_rotate(T, x->parent);
                // 设置x指针，指向根节点
                x = T->root;
            }

        } else {//当前结点是父结点的右子树
            //兄弟节点
            rbtree_node *w = x->parent->left;
            //情况1：兄弟结点为红色
            if (w->color == RED) {
                // 兄弟节点变成黑色
                w->color = BLACK;
                // 父节点变成红色
                x->parent->color = RED;
                // 父节点做右旋
                rbtree_right_rotate(T, x->parent);
                //将兄弟结点调整为父节点的左子树
                w = x->parent->left;
            }
            // 情况2：兄弟节点是黑色的，且兄弟的左孩子和右孩子都是黑色
            if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
                // 兄弟节点变成红色
                w->color = RED;
                // 轴心结点变为父节点
                x = x->parent;
            } else {
                // 情况3：x的兄弟结点是黑色的，兄弟的左孩子是黑色，右孩子是红色
                if (w->left->color == BLACK) {
                    //将右孩子变黑
                    w->right->color = BLACK;
                    //将兄弟节点变红
                    w->color = RED;
                    //对兄弟结点左旋
                    rbtree_left_rotate(T, w);
                    //将兄弟结点调整为父节点的左子树
                    w = x->parent->left;
                }
                // 情况4：x的兄弟结点是黑色的，兄弟的左孩子是红色，右孩子是黑色
                // 将兄弟节点换成父节点的颜色
                w->color = x->parent->color;
                // 把父节点和兄弟节点的左孩子变黑
                x->parent->color = BLACK;
                w->left->color = BLACK;
                // 对父节点做右旋
                rbtree_right_rotate(T, x->parent);
                //将轴心结点调整为根结点
                x = T->root;
            }
        }
    }
    // 继承节点变为黑色，为了弥补失去的黑高
    x->color = BLACK;
}

rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x) {
    while (x->left != T->nil) {
        x = x->left;
    }
    return x;
}

//找后继结点
rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x) {
    rbtree_node *y = x->parent;
    //右子树不为空，则找右子树中最左的元素
    if (x->right != T->nil) {
        return rbtree_mini(T, x->right);
    }
    //找到结点第一个父结点
    while ((y != T->nil) && (x == y->right)) {
        x = y;
        y = y->parent;
    }
    return y;
}

//覆盖结点z
//删除结点y
//轴心结点x
rbtree_node *rbtree_delete(rbtree *T, rbtree_node *z) {
    rbtree_node *y = T->nil;
    rbtree_node *x = T->nil;

    if ((z->left == T->nil) || (z->right == T->nil)) {
        y = z;//如果没有孩子或只有一个
    } else {//如果有两个子树则找后继
        y = rbtree_successor(T, z);
    }
    //一般x是y的右子树，找到轴心结点
    if (y->left != T->nil) {
        x = y->left;
    } else if (y->right != T->nil) {
        x = y->right;
    }
    //将该结点的唯一子树挂到父结点上，然后删除该结点
    x->parent = y->parent;
    if (y->parent == T->nil) {//根结点
        T->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    //进行覆盖操作
    if (y != z) {
        z->key = y->key;
        z->value = y->value;
    }
    //黑色才需要调整
    if (y->color == BLACK) {
        rbtree_delete_fixup(T, x);
    }
    return y;
}

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key) {
    rbtree_node *node = T->root;
    while (node != T->nil) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            node = node->right;
        } else {
            return node;
        }
    }
    return T->nil;
}


void rbtree_traversal(rbtree *T, rbtree_node *node) {
    if (node != T->nil) {
        rbtree_traversal(T, node->left);
        printf("key:%d, color:%d\n", node->key, node->color);
        rbtree_traversal(T, node->right);
    }
}


int main() {
    int keyArray[20] = {24, 25, 13, 35, 23, 26, 67, 47, 38, 98, 20, 19, 17, 49, 12, 21, 9, 18, 14, 15};

    rbtree *T = (rbtree *) malloc(sizeof(rbtree));
    if (T == NULL) {
        printf("malloc failed\n");
        return -1;
    }

    T->nil = (rbtree_node *) malloc(sizeof(rbtree_node));
    T->nil->color = BLACK;
    T->root = T->nil;

    rbtree_node *node = T->nil;
    int i = 0;
    for (i = 0; i < 20; i++) {
        node = (rbtree_node *) malloc(sizeof(rbtree_node));
        node->key = keyArray[i];
        node->value = NULL;
        rbtree_insert(T, node);

    }
    rbtree_traversal(T, T->root);
    printf("----------------------------------------\n");

    for (i = 0; i < 20; i++) {
        rbtree_node *node = rbtree_search(T, keyArray[i]);
        rbtree_node *cur = rbtree_delete(T, node);
        free(cur);
        rbtree_traversal(T, T->root);
        printf("----------------------------------------\n");
    }
}



