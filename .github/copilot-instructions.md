## 도구에 대한 메모

### 입력에 대해

도구를 실행할 때는 표준 입력(stdin)에 JSON 형식의 입력을 제공한다.

하지만, 모든 도구 구현이 스스로 JSON을 파싱할 여유가 있는 것은 아니다. 이러한 경우를 위해, 환경 변수로 파라미터를 아래 규칙에 따라 풀어 제공한다:

1. 환경 변수 이름은 `MEARIE__`로 시작한다. 예를 들어, 파라미터 이름이 `foo`라면, 환경 변수 이름은 `MEARIE__foo`가 된다. 이러한 형태의 이름을 "기본 이름"이라고 칭하고, 앞으로 `@`라고 줄여 표기한다.
2. 파라미터 타입이 단순하다면(예: 문자열, 정수, 숫자, 진리값, 열거형), `@`은 그 자체로 값을 갖는다. 예를 들어, `{"foo": "bar"}`라는 입력이 있다면, `MEARIE__foo` 환경 변수는 `bar`가 된다.
3. 파라미터 타입이 복잡하다면(예: 목록, 사전), `@`은 해당 부분을 JSON으로 직렬화한 문자열을 값으로 갖는다. 예를 들어, `{"foo": [1, 2, 3]}`이라는 입력이 있다면, `MEARIE__foo` 환경 변수는 `"[1, 2, 3]"`이 된다.
4. 타입이 목록인 파라미터는, `@__${index}` 형태의 추가 환경 변수를 통해 각 요소에 접근할 수 있다. 예를 들어, `{"foo": ["a", "b", "c"]}`라는 입력이 있다면, `MEARIE__foo__0`은 `a`, `MEARIE__foo__1`은 `b`, `MEARIE__foo__2`는 `c`가 된다.
5. 타입이 목록인 파라미터에는 추가로 `@__length` 환경 변수도 제공된다. 예를 들어, `{"foo": ["a", "b", "c"]}`라는 입력이 있다면, `MEARIE__foo__length`는 `3`이 된다.
6. 타입이 사전인 파라미터는, `@__${key}` 형태의 추가 환경 변수를 통해 각 값에 접근할 수 있다. 예를 들어, `{"foo": {"x": 1, "y": 2}}`라는 입력이 있다면, `MEARIE__foo__x`는 `1`, `MEARIE__foo__y`는 `2`가 된다.
7. 이러한 규칙은 중첩된 구조에도 재귀적으로 적용된다. 예를 들어, `{"foo": {"bar": [10, 20]}}`라는 입력이 있다면, `MEARIE__foo__bar__0`은 `10`, `MEARIE__foo__bar__1`은 `20`, `MEARIE__foo__bar__length`는 `2`가 된다.

아래는 실제 입력과 환경 변수 매핑의 예시이다:

```json
{
  "name": "Alice",
  "age": 30,
  "is_member": true,
  "preferences": {
    "colors": ["red", "green", "blue"],
    "notifications": {
      "email": true,
      "sms": false
    }
  }
}
```

```sh
MEARIE__name='Alice'
MEARIE__age='30'
MEARIE__is_member='true'
MEARIE__preferences='{"colors":["red","green","blue"],"notifications":{"email":true,"sms":false}}'
MEARIE__preferences__colors='["red","green","blue"]'
MEARIE__preferences__colors__length='3'
MEARIE__preferences__colors__0='red'
MEARIE__preferences__colors__1='green'
MEARIE__preferences__colors__2='blue'
MEARIE__preferences__notifications='{"email":true,"sms":false}'
MEARIE__preferences__notifications__email='true'
MEARIE__preferences__notifications__sms='false'
```

### 출력에 대해

도구는 표준 출력(stdout)에 임의의 문자열로 결과를 출력할 수 있다. 일반적으로 도구 설명에 출력 형식을 명시할 것을 권장한다.

### 매니페스트 파일에 대해

매니페스트 파일(`manifest.xml`)을 작성할 때는 아래 예시를 참조한다. 기본적으로 내용은 영어로 작성하도록 한다.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<function name="example">
    <description>
        스키마 정의를 위한 예시 함수 도구입니다.
        도구 설명은 tool에 description attrubite를 추가하거나,
        별도의 description element를 사용하여 작성할 수 있습니다.
    </description>
    <parameters>
        <parameter name="lorem" type="string">
            <description>
                파라미터 설명도 마찬가지로 attribute 또는 element로
                작성 가능합니다.
            </description>
        </parameter>
        <parameter name="ipsum" type="integer">
            <description>
                파라미터 타입은 attribute 또는 element로 작성할 수 있으며,
                attribute로 작성할 수 있는 타입은 string, integer,
                number, boolean으로 네 가지입니다.
            </description>
        </parameter>
        <parameter name="dolor">
            <description>
                element로 작성할 수 있는 파라미터 타입은
                attribute로 작성할 수 있는 타입에 더해,
                list, dictionary, enum이 있습니다.
            </description>
            <dictionary>
                <description>
                    dictionary는 item element를 여럿 포함하며,
                    item은 key attribute와 타입 attribute 또는 element로
                    구성됩니다.
                </description>
                <item key="foo" type="number">
                    <description>
                        설명은 필요한 모든 곳에 attribute 또는 element로
                        포함될 수 있습니다.
                    </description>
                </item>
                <item key="bar">
                    <description>
                        list는 설명을 갖는 타입 element를 하나 포함합니다.
                    </description>
                    <list>
                        <boolean>
                            <description>
                                list의 타입 element는
                                반환 값의 타입을 정의합니다.
                            </description>
                        </boolean>
                    </list>
                </item>
            </dictionary>
        </parameter>
        <parameter name="sit">
            <description>
                enum은 value element를 여럿 포함합니다.
                value element는 type attribute에 정의된 타입의 값만
                가질 수 있습니다.
                enum element는 description을 가질 수 없습니다.
            </description>
            <enum type="string">
                <value>hoge</value>
                <value>fuga</value>
                <value>piyo</value>
            </enum>
        </parameter>
        <parameter name="amet" type="boolean" optional="true">
            <description>
                optional attribute가 true로 설정된 파라미터는
                선택적 파라미터가 됩니다.
            </description>
        </parameter>
    </parameters>
</function>
```

XML의 텍스트 노드는 실제로 읽을 때 개행(줄바꿈)이 반영되지 않는다는 사실을 염두에 두고 작성하도록 한다.
