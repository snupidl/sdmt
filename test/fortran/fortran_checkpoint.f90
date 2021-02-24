program fortran_ex

use fortsdmt
use iso_c_binding
implicit none
integer :: i, j, k
integer :: res
integer :: dimension_numpara = 3
integer, allocatable :: dimension_format(:)
logical(1) :: c_b
character(len=25) :: config = "./config_fortran_test.xml"
character(len=14) :: seg_name = 'sdmttest_intld'
real(kind=8), pointer :: iptr (:, : , :)

integer(kind=8) :: iter

allocate(dimension_format(dimension_numpara))
allocate(iptr(3,745,1))

c_b = .false.

res = sdmt_init(config, c_b)
dimension_format(1) = 3
dimension_format(2) = 745
dimension_format(3) = 1

res = sdmt_register_segment(seg_name,  SDMT_DOUBLE, SDMT_TENSOR, dimension_numpara, dimension_format)

call c_f_pointer(sdmt_doubleptr_c(seg_name), iptr, dimension_format)



do i = 1, 3
	do j = 1, 745
		do k = 1, 1
			iptr(i,j,k) = i*100+ j*10 + k
		end do
	end do
end do

res = sdmt_start()

res = sdmt_checkpoint(1)

do i = 1, 3
	do j = 1, 745
		do k = 1, 1
			iptr(i,j,k) = 0
		end do
	end do
end do

res = sdmt_recover()

do i = 1, 3
	do j = 1, 745
		do k = 1, 1
			if (iptr(i,j,k) .ne. (i*100 + j*10 +k)) then
				print *, 'not recovered'
			end if
		end do
	end do
end do

res = sdmt_finalize()

deallocate(dimension_format)
deallocate(iptr)
end program
