# get_system_information

시스템 정보를 가져오는 함수 도구입니다.

## 입력

_(없음)_

## 출력

**JSON**:

- `system_name` (_string_): 시스템 이름입니다.
- `node_name` (_string_): 노드 이름입니다.
- `release` (_string_): 시스템 릴리스입니다.
- `version` (_string_): 시스템 버전입니다.
- `machine` (_string_): 시스템 머신입니다.
- `hostname` (_string_): 시스템 호스트 이름입니다.
- `user_uid` (_integer_): 현재 사용자의 UID입니다.
- `user_effective_uid` (_integer_): 현재 사용자의 유효 UID입니다.
- `user_gid` (_integer_): 현재 사용자의 GID입니다.
- `user_effective_gid` (_integer_): 현재 사용자의 유효 GID입니다.
- `user_name` (_optional_ _string_): 현재 사용자의 이름입니다.
- `user_home` (_optional_ _string_): 현재 사용자의 홈 디렉토리입니다.
- `user_shell` (_optional_ _string_): 현재 사용자의 로그인 셸입니다.

## 동작

`uname`과 `gethostname`, `getpwuid`를 사용하여 참고가 될 수 있는 시스템 정보를 수집한 후, JSON 형식으로 반환합니다.
