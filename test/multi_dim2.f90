program main
  integer i,j,a
  dimension a(4,3)
  do i=1,4
     do j=1,3
        a(i,j) = i*j
        print *,i*j
     end do
  end do
  do i=1,4
     do j=1,3
        print *,a(i,j)
     end do
  end do
end program main
