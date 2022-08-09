#include <stdio.h>
#include <setjmp.h>
#include <pthread.h>
#include <stdarg.h>


/* ** **** ******** **************** Thread safety **************** ******** **** ** */

#define ntyThreadLocalData                      pthread_key_t
#define ntyThreadLocalDataSet(key, value)       pthread_setspecific((key), (value))
#define ntyThreadLocalDataGet(key)              pthread_getspecific((key))
#define ntyThreadLocalDataCreate(key)           pthread_key_create(&(key), NULL)

ntyThreadLocalData ExceptionStack;

static void init_once(void) {
    ntyThreadLocalDataCreate(ExceptionStack);
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

void ntyExceptionInit(void) {
    pthread_once(&once_control, init_once);
}

/* ** **** ******** **************** try / catch / finally / throw **************** ******** **** ** */

#define EXCEPTION_MESSAGE_LENGTH                512

typedef struct _ntyException {
    const char *name;
} ntyException;

ntyException SQLException = {"SQLException"};
ntyException TimeoutException = {"TimeoutException"};


typedef struct _ntyExceptionFrame {
    jmp_buf env;

    int line;
    const char *func;
    const char *file;

    ntyException *exception;
    struct _ntyExceptionFrame *next;

    char message[EXCEPTION_MESSAGE_LENGTH + 1];
} ntyExceptionFrame;

enum {
    ExceptionEntered = 0,//0
    ExceptionThrown,    //1
    ExceptionHandled, //2
    ExceptionFinalized//3
};

#define ntyExceptionPopStack    \
    ntyThreadLocalDataSet(ExceptionStack, ((ntyExceptionFrame*)ntyThreadLocalDataGet(ExceptionStack))->next)


#define ReThrow                    ntyExceptionThrow(frame.exception, frame.func, frame.file, frame.line, NULL)
#define Throw(e, cause, ...)    ntyExceptionThrow(&(e), __func__, __FILE__, __LINE__, cause, ##__VA_ARGS__,NULL)

#define Try do {                                                                        \
            volatile int Exception_flag;                                                \
            ntyExceptionFrame frame;                                                    \
            frame.message[0] = 0;                                                       \
            frame.next = (ntyExceptionFrame*)ntyThreadLocalDataGet(ExceptionStack);     \
            ntyThreadLocalDataSet(ExceptionStack, &frame);                              \
            Exception_flag = setjmp(frame.env);                                         \
            if (Exception_flag == ExceptionEntered) {

#define Catch(nty_exception) \
                if (Exception_flag == ExceptionEntered) ntyExceptionPopStack; \
            } else if (frame.exception == &(nty_exception)) { \
                Exception_flag = ExceptionHandled;


#define Finally \
                if (Exception_flag == ExceptionEntered) ntyExceptionPopStack; \
            } { \
                if (Exception_flag == ExceptionEntered)    \
                    Exception_flag = ExceptionFinalized;
#define EndTry \
                if (Exception_flag == ExceptionEntered) ntyExceptionPopStack; \
            } if (Exception_flag == ExceptionThrown) {ReThrow;} \
            } while (0)

void ntyExceptionThrow(ntyException *excep, const char *func, const char *file, int line, const char *cause, ...) {
    va_list ap;
    ntyExceptionFrame *frame = (ntyExceptionFrame *) ntyThreadLocalDataGet(ExceptionStack);

    if (frame) {
        //异常名
        frame->exception = excep;
        frame->func = func;
        frame->file = file;
        frame->line = line;
        //异常打印的信息
        if (cause) {
            va_start(ap, cause);
            vsnprintf(frame->message, EXCEPTION_MESSAGE_LENGTH, cause, ap);
            va_end(ap);
        }

        ntyExceptionPopStack;

        longjmp(frame->env, ExceptionThrown);
    }
        //没有被catch,直接throw
    else if (cause) {
        char message[EXCEPTION_MESSAGE_LENGTH + 1];

        va_start(ap, cause);
        vsnprintf(message, EXCEPTION_MESSAGE_LENGTH, cause, ap);
        va_end(ap);
        printf("%s: %s\n raised in %s at %s:%d\n", excep->name, message, func ? func : "?", file ? file : "?", line);
    }
    else {
        printf("%s: %p\n raised in %s at %s:%d\n", excep->name, excep, func ? func : "?", file ? file : "?", line);
    }
}


/* ** **** ******** **************** debug **************** ******** **** ** */

ntyException A = {"AException"};
ntyException B = {"BException"};
ntyException C = {"CException"};
ntyException D = {"DException"};

void *thread(void *args) {
    pthread_t selfid = pthread_self();
    Try
            {
                Throw(A, "A");
            }
        Catch (A)
            {
                printf("catch A : %ld\n", selfid);
            }
    EndTry;

    Try
            {
                Throw(B, "B");
            }
        Catch (B)
            {
                printf("catch B : %ld\n", selfid);
            }
    EndTry;

    Try
            {
                Throw(C, "C");
            }
        Catch (C)
            {
                printf("catch C : %ld\n", selfid);
            }
    EndTry;

    Try
            {
                Throw(D, "D");
            }
        Catch (D)
            {
                printf("catch D : %ld\n", selfid);
            }
    EndTry;

    Try
            {
                Throw(A, "A Again");
                Throw(B, "B Again");
                Throw(C, "C Again");
                Throw(D, "D Again");
            }
        Catch (A)
            {
                printf("catch A again : %ld\n", selfid);
            }
        Catch (B)
            {
                printf("catch B again : %ld\n", selfid);
            }
        Catch (C)
            {
                printf("catch C again : %ld\n", selfid);
            }
        Catch (D)
            {
                printf("catch B again : %ld\n", selfid);
            }
    EndTry;
}


#define PTHREAD_NUM        8

int main(void) {
    ntyExceptionInit();

    printf("\n\n=> Test1: Throw\n");
    {
        Throw(D, NULL);     //ntyExceptionThrow(&(D), "_function_name_", "_file_name_", 202, ((void *) 0), ((void *) 0))
        Throw(C, "null C"); //ntyExceptionThrow(&(C), "_function_name_", "_file_name_", 203, "null C", ((void *) 0))
    }
    printf("=> Test1: Ok\n\n");


    printf("\n\n=> Test2: Try-Catch Double Nesting\n");
    {
        Try
                {
                    Try
                            {
                                Throw(B, "call B");
                            }
                        Catch (B)
                            {
                                printf("catch B \n");
                            }
                    EndTry;
                    Throw(A, NULL);
                }
            Catch(A)
                {
                    printf("catch A \n");
                    printf("Result: Ok\n");
                }
        EndTry;
    }
    printf("=> Test2: Ok\n\n");


    printf("\n\n=> Test3: Try-Catch Triple  Nesting\n");
    {
        Try
                {
                    Try
                            {

                                Try
                                        {
                                            Throw(C, "call C");
                                        }
                                    Catch (C)
                                        {
                                            printf("catch C\n");
                                        }
                                EndTry;
                                Throw(B, "call B");
                            }
                        Catch (B)
                            {
                                printf("catch B\n");
                            }
                    EndTry;
                    Throw(A, NULL);
                }
            Catch(A)
                {
                    printf("catch A\n");
                }
        EndTry;
    }
    printf("=> Test3: Ok\n\n");


    printf("=> Test4: Test Thread-safeness\n");
    int i = 0;
    pthread_t th_id[PTHREAD_NUM];

    for (i = 0; i < PTHREAD_NUM; i++) {
        pthread_create(&th_id[i], NULL, thread, NULL);
    }

    for (i = 0; i < PTHREAD_NUM; i++) {
        pthread_join(th_id[i], NULL);
    }
    printf("=> Test4: Ok\n\n");

    printf("\n\n=> Test5: No Success Catch\n");
    {
        Try
                {
                    Throw(A, "no catch A ,should Rethrow");
                }
        EndTry;
    }
    printf("=> Test5: Rethrow Success\n\n");

    printf("\n\n=> Test6: Normal Test\n");
    {
        Try
                {
                    Throw(A, "call A");
                }
            Catch(A)
                {
                    printf("catch A\n");

                }
            Finally
                {
                    printf("wxf nb\n");
                };
        EndTry;
    }
    printf("=> Test6: ok\n\n");

}

void all() {
    ///////////try
    do {
        volatile int Exception_flag;
        ntyExceptionFrame frame;
        frame.message[0] = 0;
        frame.next = (ntyExceptionFrame *) pthread_getspecific((ExceptionStack));
        pthread_setspecific((ExceptionStack), (&frame));
        Exception_flag = _setjmp(frame.env);
        if (Exception_flag == ExceptionEntered) {
            ///////////
            {
                ///////////try
                do {
                    volatile int Exception_flag;
                    ntyExceptionFrame frame;
                    frame.message[0] = 0;
                    frame.next = (ntyExceptionFrame *) pthread_getspecific((ExceptionStack));
                    pthread_setspecific((ExceptionStack), (&frame));
                    Exception_flag = _setjmp(frame.env);
                    if (Exception_flag == ExceptionEntered) {
                        ///////////
                        {
                            ///////////Throw(B, "recall B"); --->longjmp ExceptionThrown
                            ntyExceptionThrow(&(B), "_function_name_", "_file_name_", 302, "recall B", ((void *) 0));
                            ///////////
                        }
                        ///////////Catch (B)
                        if (Exception_flag == ExceptionEntered)
                            ntyExceptionPopStack;
                    }
                    else if (frame.exception == &(B)) {
                        Exception_flag = ExceptionHandled;
                        ///////////
                        {    ///////////
                            printf("recall B \n");
                            ///////////
                        }
                        ////////fin
                        if (Exception_flag == ExceptionEntered)
                            ntyExceptionPopStack;
                        if (Exception_flag == ExceptionEntered)
                            Exception_flag = ExceptionFinalized;
                        /////////////////{
                        {
                            printf("fin\n");
                        };
                        ////////////////////}
                        ///////////EndTry;
                        if (Exception_flag == ExceptionEntered)
                            ntyExceptionPopStack;
                    }
                    if (Exception_flag == ExceptionThrown)
                        ntyExceptionThrow(frame.exception, frame.func, frame.file, frame.line, ((void *) 0));
                } while (0);
                ///////////Throw(A, NULL); longjmp ExceptionThrown
                ntyExceptionThrow(&(A), "_function_name_", "_file_name_", 329, ((void *) 0), ((void *) 0));
                ///////////
            }
            ///////////Catch(A)
            if (Exception_flag == ExceptionEntered)
                ntyExceptionPopStack;
        }
        else if (frame.exception == &(A)) {
            Exception_flag = ExceptionHandled;
            ///////////
            {
                ///////////
                printf("\tResult: Ok\n");
                ///////////
            }
            /////////// EndTry;
            if (Exception_flag == ExceptionEntered)
                ntyExceptionPopStack;
        }
        if (Exception_flag == ExceptionThrown)
            ntyExceptionThrow(frame.exception, frame.func, frame.file, frame.line, ((void *) 0));
    } while (0);
    ///////////
}