module libsdmt

use iso_c_binding

ENUM, BIND(C) !:: SDMT_VT
    enumerator :: SDMT_VT = 1
    ! 4 byte integer
    enumerator :: SDMT_INT
    ! 8 byte integer
    enumerator :: SDMT_LONG
    ! 4 byte floating point
    enumerator :: SDMT_FLOAT
    ! 8 byte floating point
    enumerator :: SDMT_DOUBLE
    ! none
    enumerator :: SDMT_NUM_VT
END ENUM

ENUM, BIND(C) !:: SDMT_DT
    enumerator :: SDMT_DT = 0
    ! scalar value
    enumerator :: SDMT_SCALAR
    ! 1 dimensional array
    enumerator :: SDMT_ARRAY
    ! 2 dimensional matrix
    enumerator :: SDMT_MATRIX
    ! tensor
    enumerator :: SDMT_TENSOR
    ! none
    enumerator :: SDMT_NUM_DT
END ENUM

ENUM, BIND(C) !:: SDMT_CODE
    enumerator :: SDMT_CODE = 0
    ! the request completed successfully
    enumerator :: SDMT_SUCCESS
    ! wrong configuration
    enumerator :: SDMT_ERR_WRONG_CONFIG
    ! duplicated segment name
    enumerator :: SDMT_ERR_DUPLICATED_NAME
    ! invalid value type
    enumerator :: SDMT_ERR_WRONG_VALUE_TYPE
    ! invalid value type
    enumerator :: SDMT_ERR_WRONG_DATA_TYPE
    ! definition of dimension does not match with data type
    enumerator :: SDMT_ERR_WRONG_DIMENSION
    ! failed to allocate new memory
    enumerator :: SDMT_ERR_FAILED_ALLOCATION
    ! failed to checkpoint
    enumerator :: SDMT_ERR_FAILED_CHECKPOINT
END ENUM

contains

function sdmt_init(config, restart)
    integer(kind(SDMT_CODE)) :: sdmt_init
    character(len=*) :: config
    logical :: restart

    sdmt_init = sdmt_init_c(config, restart)
end function

function sdmt_register_segment(sname, vt, dt, sdim)
    integer(kind(SDMT_CODE)) :: sdmt_register_segment
    character(len = *) :: sname
    integer(kind(SDMT_VT)) :: vt
    integer(kind(SDMT_DT)) :: dt
    integer, dimension(:) :: sdim

    sdmt_register_segment = sdmt_register_segment_c(sname, vt, dt, sdim)
end function

function sdmt_intptr(sname)
    integer, pointer :: sdmt_intptr
    character(len = *) sname

    sdmt_intptr = sdmt_intptr_c(sname)
end function

function sdmt_start()
    integer(kind(SDMT_CODE)) :: sdmt_start

    sdmt_start = sdmt_start_c()
end function

function sdmt_checkpoint(lev)
    integer(kind(SDMT_CODE)) :: sdmt_checkpoint
    integer :: lev

    sdmt_checkpoint = sdmt_checkpoint_c(lev)
end function

function sdmt_recover()
    integer(kind(SDMT_CODE)) :: sdmt_recover

    sdmt_recover = sdmt_recover_c()
end function

function sdmt_finalize()
    integer :: sdmt_finalize

    sdmt_finalize = sdmt_finalize_c()
end function

end module
