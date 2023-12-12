// RFID 바교 함수
// RFID의 UID는 element 10, byte 배열
// 배열 인덱스 0~9까지 비교하여 하나라도 겹치는게 없다면 0, 모두 겹치면 1 리턴
bool bytecmp(byte* b1, byte* b2){ 
  for(int i = 0 ; i <10 ; i++){
    if(b1[i] != b2[i])
      return 0;
  }
  return 1;
}

typedef struct _Node{
    byte data[10] = {0x00,};
    struct _Node* next;
}Node;
Node* head;

void listinit(){
    head = NULL;
}

void insert(byte* data){
    Node* ptr;
    Node* newNode = (Node*)malloc(sizeof(Node)); 
    for(int i = 0 ; i < 10 ; i++){
      newNode->data[i] = data[i];
      }    // 데이터 할당 
    newNode->next = NULL;    // next 포인터 초기화 
    
    if(head == NULL){    // 첫 번째 노드 생성
        head = newNode;
    }else{
		// not empty, 가장 앞에 노드 추가 
        if(head->data > newNode->data){    
            newNode->next = head;
            head = newNode;
            return;
        }
		 // 중간에 노드 추가 
        for(ptr = head; ptr->next; ptr=ptr->next){   
            if(ptr->data < newNode->data && ptr->next->data > newNode->data){
                newNode->next = ptr->next;
                ptr->next = newNode;
                return;
            }
        }
        
        ptr->next = newNode;    // 마지막에 노드 추가  
    }
    
}
int deleteNode(byte* data){
    Node *cur, *prev;
    cur = prev = head;
    
    if(head == NULL){    // empty list 
        printf("error: list is empty!\n");
        return 0;
    }        
    
    if(bytecmp(head->data, data)){    // 가장 앞의 노드 삭제
        head = cur->next;
        cur->next = NULL;
        free(cur);
        return 1;
    }
    
    for(; cur; cur= cur->next){    // 중간 또는 마지막 노드 삭제
        if(bytecmp(cur->data, data)){
            prev->next = cur->next;
            cur->next = NULL;
            free(cur);
            return 1;
        }
        prev = cur;
    }
    
    return 0;    // 해당 데이터가 리스트에 없음 
}

bool search_list(byte* data){
    Node* ptr;

    for(ptr = head ; ptr ; ptr=ptr->next){
        if(bytecmp(ptr->data, data))
          return 1; //데이터 발견 O
    }
    return 0; // 데이터 발견 X
}
