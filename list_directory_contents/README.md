# list_directory_contents

## 입력

**JSON**:

- `directory_path` (_string_): 내용을 나열할 디렉터리의 경로입니다. 절대 경로만 허용됩니다.
- `recursive` (_optional_ _boolean_): 하위 디렉터리의 내용을 재귀적으로 나열할지 여부입니다. 기본값은 `false`입니다.

## 출력

**JSON**:

- `path` (_string_): 나열된 디렉터리의 절대 경로입니다.
- `name` (_string_): 나열된 디렉터리의 이름입니다.
- `permissions` (_string_): 나열된 디렉터리의 권한입니다. (예: `rwxr-xr-x`)
- `owner` (_integer_ | _string_): 나열된 디렉터리의 소유자입니다.
- `group` (_integer_ | _string_): 나열된 디렉터리의 그룹입니다.
- `modified_at` (_string_): 나열된 디렉터리의 마지막 수정 시간입니다. (ISO 8601 형식)
- `contents` (_object_): 나열된 디렉터리의 내용입니다.
  - `directories` (_array\[object\]_) : 디렉터리 목록입니다.
    - `path` (_string_): 디렉터리의 절대 경로입니다.
    - `name` (_string_): 디렉터리 이름입니다.
    - `permissions` (_string_): 디렉터리 권한입니다. (예: `rwxr-xr-x`)
    - `owner` (_integer_ | _string_): 디렉터리 소유자입니다.
    - `group` (_integer_ | _string_): 디렉터리 그룹입니다.
    - `modified_at` (_string_): 디렉터리의 마지막 수정 시간입니다. (ISO 8601 형식)
    - `contents` (_optional_ _object_): 디렉터리의 내용입니다. 재귀적으로 나열할 때만 포함됩니다.
      - ... (상위와 동일한 구조의 `directories`, `files`, `links` 포함)
  - `files` (_array\[object\]_) : 파일 목록입니다.
    - `path` (_string_): 파일의 절대 경로입니다.
    - `name` (_string_): 파일 이름입니다.
    - `permissions` (_string_): 파일 권한입니다. (예: `rw-r--r--`)
    - `owner` (_integer_ | _string_): 파일 소유자입니다.
    - `group` (_integer_ | _string_): 파일 그룹입니다.
    - `modified_at` (_string_): 파일의 마지막 수정 시간입니다. (ISO 8601 형식)
    - `size` (_integer_): 파일 크기입니다. (바이트 단위)
  - `links` (_array\[object\]_) : 심볼릭 링크 목록입니다.
    - `path` (_string_): 심볼릭 링크의 절대 경로입니다.
    - `name` (_string_): 심볼릭 링크 이름입니다.
    - `permissions` (_string_): 심볼릭 링크 권한입니다. (예: `rwxrwxrwx`)
    - `owner` (_integer_ | _string_): 심볼릭 링크 소유자입니다.
    - `group` (_integer_ | _string_): 심볼릭 링크 그룹입니다.
    - `modified_at` (_string_): 심볼릭 링크의 마지막 수정 시간입니다. (ISO 8601 형식)
    - `target_path` (_string_): 심볼릭 링크가 가리키는 대상 경로입니다.
    - `target_type` (_string_): 심볼릭 링크가 가리키는 대상의 유형입니다. (`file`, `directory`, `link` 중 하나)

## 동작

지정된 디렉터리의 내용을 나열하여 JSON 형식으로 반환합니다. `recursive` 옵션이 `true`로 설정된 경우, 하위 디렉터리의 내용도 재귀적으로 나열하여 포함합니다.
