# Внешняя компонента 1С для генерации JWT токенов.

Формирование токена происходить на базе сертификата подписанного по алгоритму ES256.
Компонента решает проблему передачи числовых параметров json при формировании токена. Платформа 1С конвертирует их в строки.


# Зависимости

Для работы компоненты необходим уустановленный OpenSSL. Скачать его можно по ссылке [www.openssl.org/source/old/1.1.1/openssl-1.1.1k.tar.gz](https://www.openssl.org/source/old/1.1.1/openssl-1.1.1k.tar.gz).
Для сборки openssl также необходим Perl. Скачать можно по ссылке [strawberryperl.com/](https://github.com/StrawberryPerl/Perl-Dist-Strawberry/releases/download/SP_5380_5361/strawberry-perl-5.38.0.1-64bit.msi).
Так же для сборки необходима библиотека [jwt-cpp](https://github.com/Thalhammer/jwt-cpp/tree/master).


# Сборка и установка.

Для сборки использовался Visual Studio community 2019.

## Сборка и установка openssl

выполняется в среде x64 Native Tools Command Prompt for VS 2019, запущенного от имени администратора

```
perl Configure VC-WIN64A
nmake
nmake install
```

При успешном выполнении в Programm Files создается каталог OpenSSL, в котором необходимые папки для дальнейшей сборки проекта (`include` и `lib`).

## Сборка и установка jwt-cpp

```
md build
cd build
cmake -G "Visual Studio 16 2019" ..
start JWT.sln
```

В Visual Studio:

    1.  Откройте "Свойства проекта" (Project Properties).

    2.  В разделе "С/С++" (Additional Include Directories) добавьте путь к заголовкам OpenSSL. Обычно это C:\Program Files\OpenSSL\include (замените на ваш путь).

    3.  В разделе "Компоновщик" (Linker) -> "Общие" (General) добавьте путь к библиотекам OpenSSL в "Дополнительные каталоги библиотек" (Additional Library Directories). Обычно это C:\Program Files\OpenSSL\lib (замените на ваш путь).

    4. В разделе "Компоновщик" (Linker) -> "Ввод" (Input) добавьте libcrypto.lib и libssl.lib в "Дополнительные зависимости" (Additional Dependencies).

После этого выполните сборку проекта.


## Сборка и установка 1c-jwt-token

    1.  Откройте проект в Visual Studio community 2019.
    2.  Откройте "Свойства проекта" (Project Properties).
    3.  В разделе "Компоновщик" (Linker) -> "Общие" (General) добавьте путь к библиотекам OpenSSL в "Дополнительные каталоги библиотек" (Additional Library Directories). Обычно это C:\Program Files\OpenSSL\lib (замените на ваш путь).
    4. В разделе "Компоновщик" (Linker) -> "Ввод" (Input) добавьте libcrypto.lib и libssl.lib в "Дополнительные зависимости" (Additional Dependencies).
    5. В разделе "С/С++" (Additional Include Directories) добавьте пути к заголовкам OpenSSL, jwt-cpp и данного проекта. Обычно это C:\Program Files\OpenSSL\include, ...\jwt-cpp\include, ...\include. 

После этого пересоберите проект. Если все пверно настроено 1cJWTtoken.dll готово.


# Использование

В 1С используйте следующий код для подключения компоненты:
```
Рез = ПодключитьВнешнююКомпоненту(ПутьФайла, "JWT", ТипВнешнейКомпоненты.Native);
ОбъектКомпоненты = Новый("AddIn.JWT.1c-jwt-token");
ВК = ОбъектКомпоненты;
ВК.Включить();
Ответ = ВК.Включен;
Если Ответ Тогда
    Токен = ВК.СформироватьТокен(cert , merchant_id, shop_id, pub_key_id);
КонецЕсли;
```

    `cert` - строка сертификата для подписи токена;

    `merchant_id`, `shop_id`, `pub_key_id` - Числовые значения, необходимые для формирования токена;
