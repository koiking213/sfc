program main
  integer i,a
  dimension a(4,3)
  do i=1,4
     a(i,3) = i
  end do
  do i=1,4
     print *,a(i,3)
  end do
end program main
