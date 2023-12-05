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
    Токен = ВК.СформироватьТокен(ident , merchant_id, shop_id, pub_key_id, sum, currency, card_id, cardholder_id, sbp);
КонецЕсли;
```

    `ident` - строка json. обязательно должна содержать cert (сертификата для подписи токена) и key (уникальный идентифйикатор строкой) ;

    `merchant_id`, `shop_id`, `pub_key_id` - Числовые значения, необходимые для формирования токена;

    `sum` - сумма платежа;

    `currency` - валюта платежа ("RUB");

    `card_id` - id карты для выплаты, если нет то "";

    `cardholder_id` - id владельца карты, если нет то "";

    `sbp` - строка json. обязательно должна содержать bank_member_id (id банка в системе быстрых платежей) и phone (телефон получателя платежа)


# Установка на prod.

    Для установки необходимо выполнить установку ssl. Для этого в проекте находится скомпилированная версия openssl-1.1.1k. 

        Разархивировать файл ssl-1.1.1k-x64.tar.gz

        Скопировать каталог `OpenSSL` в `C:\Program Files\` и каталог `SSL` в `C:\Program Files\Common Files\`

        Открыть "File Explorer" -> правой кнопкой мыши на "This PC" -> щелкнуть на "Properties" -> щелкнуть на "Advanced System Settings"

        Щелкнуть на "Environment variables" на вкладке "Advanced" -> щелкнуть "New" and Write в "Variable Name", "OPENSSLDIR", и в "Variable Value", "C:\Program Files\Common Files\SS"

        Щелкнуть на "Environment variables" на вкладке "Advanced" -> щелкнуть "New" and Write в "Variable Name", "ENGINESDIR", и в "Variable Value", "C:\Program Files\OpenSSL\lib\engines-1_1"

        В "System Variables" найти "Path", и добавить эти две папки: "C:\Program Files\Common Files\SS" и "C:\Program Files\OpenSSL\lib\engines-1_1", а так же добавить путь к каталогу с испольняемыми файлами "C:\Program Files\OpenSSL\bin"

        Что бы проверить корректность установки openssl в терминале введите комманду `openssl version`;

    Из каталога `x64\Release\` или `x64\Debug\` (в зависимости от текущего способа сборки) скопировать файл `1cJWTtoken.pdb` в каталог `C:\Users\{ИмяПользователя}\AppData\Roaming\1C\1cv8\ExtCompT\`;

    Поместить компоненту в рабочий каталог.
