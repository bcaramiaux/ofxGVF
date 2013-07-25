plotter <- function(gid,rt){
par(mfrow=c(3,1),mar=c(2,8,4,2)+0.1)
 g1 <- scan("g1.txt")
 g2 <- scan("g2.txt")
 g3 <- scan("g3.txt")
 g1y <- scan("g1y.txt")
 g2y <- scan("g2y.txt")
 g3y <- scan("g3y.txt")
 p1 <- scan("p1.txt")
 p2 <- scan("p2.txt")
 p3 <- scan("p3.txt")
 tot <- scan("tot.txt")
 ug <- scan("ug.txt")
 ugy<- scan("ugy.txt")
 w1 <- scan("w1.txt")
 w2 <- scan("w2.txt")
 w3 <- scan("w3.txt")
 rs <- scan("rs.txt")
 phase<-scan("phase.txt")
 phase2<-scan("phase2.txt")
 phase3<-scan("phase3.txt")
 v <- c(g1,g2,g3,g1y,g2y,g3y,ug,ugy)
 title1 <- sprintf("RT = %d Input Gesture = %s",rt,gid)
 plot(0,type="l",ylim=c(min(v),max(v)),xlim=c(0,length(tot)),main=title1)
    legend("bottomright", c("t1","t2","t3","user"), cex=0.8, col=c("red","pink","green","blue"), 
    lty=1:3, lwd=2, bty="n");  
   lines(g2,lty=1,col="red")
   lines(g3,lty=1,col="pink")
   lines(g1y,lty=1,col="green")
   lines(g2y,lty=1,col="red")
   lines(g3y,lty=1,col="pink")
   lines(ug,lty=3,col="blue")
   lines(ugy,lty=3,col="blue")
 plot(tot/2000,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="indv Prob",col="black",ylab="")
   legend("topright", c("t1","t2","t3","total"), cex=0.8, col=c("red","pink","green","black"), 
    lty=1:3, lwd=2, bty="n");  
   lines(w1/2000,lty=1,col="green")
   lines(w2/2000,lty=1,col="red")
   lines(w3/2000,lty=1,col="pink")
 plot(0,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="phase")
   points((rs/9)*1-0.3,lty=5,col="orange",lwd=1,pch=20)>   lines(phase,lty=5,col="green")
   lines(phase2,lty=5,col="red")
   lines(phase3,lty=5,col="pink")
}





batchplotter <- function(num){
dag <-  "/Users/thomasrushmore/Desktop/data/run"
pics <- "~/Desktop/data/screenshots/"
rta <- c(0,500,100)
for(i in 1:num){
for(j in 1:3){

workdir <- paste(dag,as.character(((i-1)*3)+j),sep="")
workdir <- paste(workdir,"/",sep="")
print(i)
print(workdir)
setwd(workdir)
photdir <- paste(pics,as.character(((i-1)*3)+j),sep="")
photodir <- paste(photdir,".pdf",sep="")
pdf(photodir)
plotter(i,rta[j]);
dev.off()

}
}
}