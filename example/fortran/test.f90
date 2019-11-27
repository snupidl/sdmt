program test

use libsdmt
implicit none


integer :: i
integer, dimension(:), pointer :: iptr
integer :: s_dim(1)

print *, '[fortran] sdmt_init\n'
print *, sdmt_init('../../../config.xml', .false.)

s_dim(1) = 1024
print *, '[fortran] sdmt_register_segment\n'
print *, sdmt_register_segment('sdmttest_int1d', SDMT_INT, SDMT_ARRAY, s_dim)

print *, '[fortran] sdmt_intptr\n'
iptr = sdmt_intptr('sdttest_int1d')

do i = 1, 1024
    iptr(i) = i * i
end do

print *, '[fortran] sdmt_start\n'
print *, sdmt_start()

print *, '[fortran] sdmt_checkpoint\n'
print *, sdmt_checkpoint(1)

do i = 1, 1024
    iptr(i) = 0
end do

print *, '[fortran] sdmt_recover\n'
print *, sdmt_recover()

do i = 1, 1024
!    if iptr(i) .ne. (i * i) then
!        print *, 'not recovered'
!    end if
    print *, iptr(i)
end do

print *, '[fortran] sdmt_finalize\n'
print *, sdmt_finalize()

end program
