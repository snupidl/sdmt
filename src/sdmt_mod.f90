module fortsdmt

use iso_c_binding

interface

function sdmt_init_c(config, restart) bind (C, name="sdmt_init_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_init_c
character(kind=c_char) :: config(*)
logical(c_bool) :: restart
end function

function sdmt_start_c() bind (C , name = "sdmt_start_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_start_c
end function

function sdmt_finalize_c() bind (C, name = "sdmt_finalize_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_finalize_c
end function
					
function sdmt_register_snapshot_c(sname, vt, dt, dim_numpara, dim_format) bind (C, name = "sdmt_register_snapshot_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_register_snapshot_c
character(kind=c_char) :: sname(*)
integer(c_int) :: vt, dt, dim_numpara
integer(c_int) :: dim_format(dim_numpara)
end function

function sdmt_register_int_parameter_c(sname, val) bind (C, name = "sdmt_register_int_parameter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_register_int_parameter_c
character(kind=c_char) :: sname(*)
integer(c_int) :: val
end function

function sdmt_get_int_parameter_c(sname) bind (C, name = "sdmt_get_int_parameter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_get_int_parameter_c
character(kind=c_char) :: sname(*)
end function

function sdmt_register_long_parameter_c(sname, val) bind (C, name = "sdmt_register_long_parameter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_register_long_parameter_c
character(kind=c_char) :: sname(*)
integer(c_int) :: val
end function

function sdmt_get_long_parameter_c(sname) bind (C, name = "sdmt_get_long_parameter_c_")
use iso_c_binding
implicit none
integer(c_long) :: sdmt_get_long_parameter_c
character(kind=c_char) :: sname(*)
end function

function sdmt_register_float_parameter_c(sname, val) bind (C, name = "sdmt_register_float_parameter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_register_float_parameter_c
character(kind=c_char) :: sname(*)
real(c_float) :: val
end function

function sdmt_get_float_parameter_c(sname) bind (C, name = "sdmt_get_float_parameter_c_")
use iso_c_binding
implicit none
real(c_float) :: sdmt_get_float_parameter_c
character(kind=c_char) :: sname(*)
end function

function sdmt_register_double_parameter_c(sname, val) bind (C, name="sdmt_register_double_parameter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_register_double_parameter_c
character(kind=c_char) :: sname(*)
real(c_double) :: val
end function

function sdmt_get_double_parameter_c(sname) bind (C, name="sdmt_get_double_parameter_c_")
use iso_c_binding
implicit none
real(c_double) :: sdmt_get_double_parameter_c
character(kind=c_char) :: sname(*)
end function

function sdmt_checkpoint_c(lev) bind (C, name="sdmt_checkpoint_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_checkpoint_c
integer(c_int) :: lev
end function

function sdmt_recover_c() bind (C, name="sdmt_recover_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_recover_c
end function

function sdmt_exist_c(sname) bind (C, name="sdmt_exist_c_")
use iso_c_binding
implicit none
logical(c_bool) :: sdmt_exist_c
character(kind=c_char) :: sname(*)
end function

function sdmt_get_snapshot_c(sname) bind (C, name="sdmt_get_snapshot_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_get_snapshot_c
character(kind=c_char) :: sname(*)
end function

function sdmt_iter_c() bind (C, name="sdmt_iter_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_iter_c
end function

function sdmt_next_c() bind (C, name="sdmt_next_c_")
use iso_c_binding
implicit none
integer(c_int) :: sdmt_next_c
end function

function sdmt_comm_c() bind (C, name="sdmt_comm_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_comm_c
end function

function sdmt_intptr_c(sname) bind (C,name="sdmt_intptr_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_intptr_c
character(kind=c_char) :: sname(*)
end function

function sdmt_longptr_c(sname) bind (C,name="sdmt_longptr_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_longptr_c
character(kind=c_char) :: sname(*)
end function

function sdmt_floatptr_c(sname) bind (C,name="sdmt_floatptr_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_floatptr_c
character(kind=c_char) :: sname(*)
end function

function sdmt_doubleptr_c(sname) bind (C,name="sdmt_doubleptr_c_")
use iso_c_binding
implicit none
type(c_ptr) :: sdmt_doubleptr_c
character(kind=c_char) :: sname(*)
end function

end interface

public::SDMT_INT, SDMT_LONG 
public::SDMT_FLOAT, SDMT_DOUBLE, SDMT_NUM_VT
public::SDMT_SCALAR, SDMT_ARRAY, SDMT_MATRIX
public::SDMT_TENSOR, SDMT_NUM_DT
public::SDMT_SUCCESS, SDMT_ERR_WRONG_CONFIG
public::SDMT_ERR_DUPLICATED_NAME, SDMT_ERR_WRONG_VALUE_TYPE
public::SDMT_ERR_WRONG_DATA_TYPE, SDMT_ERR_WRONG_DIMENSION
public::SDMT_ERR_FAILED_ALLOCATION, SDMT_ERR_FAILED_CHECKPOINT


public::sdmt_init
public::sdmt_start 
public::sdmt_finalize, sdmt_register_snapshot
public::sdmt_register_int_parameter, sdmt_get_int_parameter
public::sdmt_register_long_parameter, sdmt_get_long_parameter
public::sdmt_register_float_parameter, sdmt_get_float_parameter
public::sdmt_register_double_parameter, sdmt_get_double_parameter
public::sdmt_checkpoint, sdmt_recover
public::sdmt_exist, sdmt_get_snapshot
public::sdmt_iter, sdmt_next
public::sdmt_comm
public::sdmt_intptr
public::sdmt_longptr, sdmt_floatptr
public::sdmt_doubleptr

ENUM, BIND(C)
ENUMERATOR :: SDMT_INT
ENUMERATOR :: SDMT_LONG
ENUMERATOR :: SDMT_FLOAT
ENUMERATOR :: SDMT_DOUBLE
ENUMERATOR :: SDMT_NUM_VT
END ENUM
ENUM, BIND(C) 
ENUMERATOR :: SDMT_SCALAR
ENUMERATOR :: SDMT_ARRAY
ENUMERATOR :: SDMT_MATRIX
ENUMERATOR :: SDMT_TENSOR
ENUMERATOR :: SDMT_NUM_DT
END ENUM
ENUM, BIND(C)
ENUMERATOR :: SDMT_SUCCESS
ENUMERATOR :: SDMT_ERR_WRONG_CONFIG
ENUMERATOR :: SDMT_ERR_DUPLICATED_NAME
ENUMERATOR :: SDMT_ERR_WRONG_VALUE_TYPE
ENUMERATOR :: SDMT_ERR_WRONG_DATA_TYPE
ENUMERATOR :: SDMT_ERR_WRONG_DIMENSION
ENUMERATOR :: SDMT_ERR_FAILED_ALLOCATION
ENUMERATOR :: SDMT_ERR_FAILED_CHECKPOINT
END ENUM

contains


function sdmt_init(config, restart)
implicit none
integer :: sdmt_init
character(len=*) :: config
logical(1) :: restart
sdmt_init = sdmt_init_c(config, restart)
end function

function sdmt_start()
implicit none
integer :: sdmt_start
sdmt_start = sdmt_start_c()
end function

function sdmt_finalize()
implicit none
integer :: sdmt_finalize
sdmt_finalize = sdmt_finalize_c()
end function

function sdmt_register_snapshot(sname, vt, dt, dim_numpara, dim_format)
implicit none
integer :: sdmt_register_snapshot
character(len=*) :: sname
integer :: vt, dt
integer ::dim_numpara
integer, optional :: dim_format(dim_numpara)
sdmt_register_snapshot = sdmt_register_snapshot_c(sname, vt, dt, dim_numpara, dim_format)
end function

function sdmt_register_int_parameter(sname, val)
implicit none
integer :: sdmt_register_int_parameter
character(len=*) :: sname
integer :: val
sdmt_register_int_parameter=sdmt_register_int_parameter_c(sname,val)
end function

function sdmt_get_int_parameter(sname)
implicit none
integer :: sdmt_get_int_parameter
character(len=*) :: sname
sdmt_get_int_parameter = sdmt_get_int_parameter_c(sname)
end function

function sdmt_register_long_parameter(sname, val)
implicit none
integer :: sdmt_register_long_parameter
character(len=*) :: sname
integer :: val
sdmt_register_long_parameter = sdmt_register_long_parameter_c(sname,val)
end function

function sdmt_get_long_parameter(sname)
implicit none
integer(kind=8) :: sdmt_get_long_parameter
character(len=*) :: sname
sdmt_get_long_parameter = sdmt_get_long_parameter_c(sname)
end function

function sdmt_register_float_parameter(sname, val)
implicit none
integer :: sdmt_register_float_parameter
character(len=*) :: sname
real :: val
sdmt_register_float_parameter = sdmt_register_float_parameter_c(sname, val)
end function

function sdmt_get_float_parameter(sname)
implicit none
real :: sdmt_get_float_parameter
character(len=*) :: sname
sdmt_get_float_parameter = sdmt_get_float_parameter_c(sname)
end function

function sdmt_register_double_parameter(sname, val)
implicit none
integer :: sdmt_register_double_parameter
character(len=*) :: sname
real(kind=8) :: val
sdmt_register_double_parameter = sdmt_register_double_parameter_c(sname, val)
end function

function sdmt_get_double_parameter(sname)
implicit none
real(kind=8) :: sdmt_get_double_parameter
character(len=*) :: sname
sdmt_get_double_parameter = sdmt_get_double_parameter_c(sname)
end function

function sdmt_checkpoint(lev)
implicit none
integer :: sdmt_checkpoint
integer :: lev
sdmt_checkpoint = sdmt_checkpoint_c(lev)
end function

function sdmt_recover()
implicit none
integer :: sdmt_recover
sdmt_recover = sdmt_recover_c()
end function

function sdmt_exist(sname)
implicit none
logical(kind=1) :: sdmt_exist
character(len=*) :: sname
sdmt_exist = sdmt_exist_c(sname)
end function

function sdmt_get_snapshot(sname) 
implicit none
type(c_ptr) :: sdmt_get_snapshot
character(len=*) :: sname
sdmt_get_snapshot = sdmt_get_snapshot_c(sname)
end function

function sdmt_iter()
implicit none
integer(kind=8) :: sdmt_iter
sdmt_iter = sdmt_iter_c()
end function

function sdmt_next()
implicit none
integer(kind=8) :: sdmt_next
sdmt_next = sdmt_next_c()
end function

function sdmt_comm()
implicit none
type(c_ptr) :: sdmt_comm
sdmt_comm = sdmt_comm_c()
end function

!subroutine sdmt_intptr(fptr, sname,  sz)
!implicit none
!character(len=*) :: sname
!integer, dimension(:), pointer :: fptr(:)
!integer :: sz
!call c_f_pointer(sdmt_intptr_c(sname),fptr, shape=[sz])
!end subroutine

!subroutine sdmt_longptr(fptr, sname, sz)
!implicit none
!character(len=*) :: sname
!integer(kind=8), dimension(:), pointer :: fptr
!integer :: sz
!call c_f_pointer(sdmt_longptr_c(sname), fptr, shape=[sz])
!end subroutine

!subroutine sdmt_floatptr(fptr, sname, sz)
!implicit none
!character(len=*) :: sname
!real, dimension(:), pointer :: fptr
!integer :: sz
!call c_f_pointer(sdmt_floatptr_c(sname), fptr, shape=[sz])
!end subroutine

!subroutine sdmt_doubleptr(fptr, sname, sz)
!implicit none
!character(len=*) :: sname
!real(kind=8), dimension(:), pointer :: fptr
!integer :: sz
!call c_f_pointer(sdmt_doubleptr_c(sname), fptr, shape=[sz])
!end subroutine

!subroutine sdmt_intmat(fptr, sname, dm)
!implicit none
!character(len=*) :: sname
!integer, pointer :: fptr(:,:,:)
!integer, dimension(3) :: dm
!call c_f_pointer(sdmt_intptr_c(sname), !end subroutine

end module
