#include <iostream>
#include <memory>

#include "User.hpp" // Вместе с этим файлом Clang теперь автоматически загрузит User-odb.hxx

int main() {
    // Код main остается абсолютно чистым, без изменений...
    std::shared_ptr<User> user = std::make_shared<User>("Дмитрий", "dima@mail.com");
    user->save<User>(); 
    
    unsigned long saved_id = user->id; 
    std::cout << "Пользователь сохранен с ID: " << saved_id << std::endl;
    return 0;
}