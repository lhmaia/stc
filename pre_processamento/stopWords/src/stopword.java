import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.HashSet;
import java.util.StringTokenizer;


public class stopword {

	
	public static void main(String[] args) {
		
		if (args.length != 3) {
			System.out.println();
			System.out.println("Opcoes invalidas. Sintaxe correta:");
			System.out.println("                  stopword <lista_stopwords> <arquivo_entrada> <arquivo_saida>");
			System.out.println();
			System.exit(1);
		}
		
		String nome_arq_lista = args[0];
		String nome_arq_entrada = args[1];
		String nome_arq_saida = args[2];
		
		
		File input = new File(nome_arq_lista);
			
		HashSet<String> hash_words = new HashSet<String>();
		
		try{ 
			//lendo lista de stop words e inserindo em hashset 
			BufferedReader bf_input = new BufferedReader(new FileReader(input));
			String linha;
			
			while((linha = bf_input.readLine()) != null){
				StringTokenizer st = new StringTokenizer(linha);
				
				while (st.hasMoreElements()){
					hash_words.add(st.nextToken().toLowerCase());
				}
				
			}
			bf_input.close();
			
			input = new File(nome_arq_entrada);
			bf_input = new BufferedReader(new FileReader(input));
			
			FileWriter output = new FileWriter(nome_arq_saida);
			
			while((linha = bf_input.readLine()) != null){
				StringTokenizer st = new StringTokenizer(linha);
				
				while (st.hasMoreElements()){//testa se esta na lista de stopwords
					String token = st.nextToken().toLowerCase();
					
					//retirando acentos
					token = token.replaceAll("[ãâàáä]", "a")
                    		.replaceAll("[êèéë]", "e")
		                    .replaceAll("[îìíï]", "i")
		                    .replaceAll("[õôòóö]", "o")
		                    .replaceAll("[ûúùü]", "u")
		                    .replace('ç', 'c')
		                    .replace('ñ', 'n');	
					
					//retirando pontuacao indesejada
					token = token.replaceAll("[$%#`^~|(){}'\"*,]", " ")
							.replaceAll("[!?:;]", ".");
					
					if (!hash_words.contains(token)){//retira se esta na lista de stopwords
						if (token.length() < 4 || (!(token.substring(0, 4).equalsIgnoreCase("http")) 
								&& !(token.substring(0, 3).equalsIgnoreCase("www")))){//retira enderecos web
							//System.out.print(token);
							//System.out.print(" ");
							output.write(token);
							output.write(" ");
						}//fim if retira enderecos web
						else{
							if(token.length() >= 4) {
								output.write(".");
								output.write(" ");
							}
						}
					}//fim if testa se esta na lista stopwords
					
				}
				//System.out.println();
				output.write('\n');
			}	
			bf_input.close();
			output.close();
		}
		catch (Exception e){
			e.printStackTrace();
		}
		

	}

}
